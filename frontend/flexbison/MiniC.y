%{
#include <cstdio>
#include <cstring>

// 词法分析头文件
#include "FlexLexer.h"

// bison生成的头文件
#include "BisonParser.h"

// 抽象语法树函数定义原型头文件
#include "AST.h"

#include "IntegerType.h"

// LR分析失败时所调用函数的原型声明
void yyerror(char * msg);

// 用于生成唯一标签的计数器
static int unique_label_counter = 0;

%}

// 联合体声明，用于后续终结符和非终结符号属性指定使用
%union {
    class ast_node * node;

    struct digit_int_attr integer_num;
    struct digit_real_attr float_num;
    struct var_id_attr var_id;
    struct type_attr type;
    int op_class;
};

// 文法的开始符号
%start  CompileUnit

// 指定文法的终结符号，<>可指定文法属性
// 对于单个字符的算符或者分隔符，在词法分析时可直返返回对应的ASCII码值，bison预留了255以内的值
// %token开始的符号称之为终结符，需要词法分析工具如flex识别后返回
// %type开始的符号称之为非终结符，需要通过文法产生式来定义
// %token或%type之后的<>括住的内容成为文法符号的属性，定义在前面的%union中的成员名字。
%token <integer_num> T_DIGIT
%token <var_id> T_ID
%token <type> T_INT T_VOID

// 关键或保留字 一词一类 不需要赋予语义属性
%token T_RETURN T_IF T_ELSE T_WHILE T_BREAK T_CONTINUE

// 分隔符 一词一类 不需要赋予语义属性
%token T_SEMICOLON T_L_PAREN T_R_PAREN T_L_BRACE T_R_BRACE
%token T_L_BRACKET T_R_BRACKET
%token T_COMMA

// 运算符
%token T_ASSIGN T_SUB T_ADD T_MUL T_DIV T_MOD
%token T_GT T_GE T_LT T_LE T_EQ T_NE
%token T_AND T_OR T_NOT

// 非终结符
// %type指定文法的非终结符号，<>可指定文法属性
%type <node> CompileUnit
%type <node> FuncDef
%type <node> Block
%type <node> BlockItemList
%type <node> BlockItem
%type <node> Statement
%type <node> Expr
%type <node> LVal
%type <node> VarDecl VarDeclExpr VarDef
%type <node> AddExp UnaryExp PrimaryExp
%type <node> MulExp
%type <node> RealParamList
%type <node> RelExp
%type <type> BasicType
%type <op_class> AddOp MulOp RelOp
%type <node> AndExpr
%type <node> OrExpr
%type <node> FormalParamList
%type <node> FormalParam
%type <node> ArrayDimensions
%type <node> ArrayAccess
%%

// 编译单元可包含若干个函数与全局变量定义。要在语义分析时检查main函数存在
// compileUnit: (funcDef | varDecl)* EOF;
// bison不支持闭包运算，为便于追加修改成左递归方式
// compileUnit: funcDef | varDecl | compileUnit funcDef | compileUnit varDecl
CompileUnit : FuncDef {

		// 创建一个编译单元的节点AST_OP_COMPILE_UNIT
		$$ = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT, $1);

		// 设置到全局变量中
		ast_root = $$;
	}
	| VarDecl {

		// 创建一个编译单元的节点AST_OP_COMPILE_UNIT
		$$ = create_contain_node(ast_operator_type::AST_OP_COMPILE_UNIT, $1);
		ast_root = $$;
	}
	| CompileUnit FuncDef {

		// 把函数定义的节点作为编译单元的孩子
		$$ = $1->insert_son_node($2);
	}
	| CompileUnit VarDecl {
		// 把变量定义的节点作为编译单元的孩子
		$$ = $1->insert_son_node($2);
	}
	;

// 函数定义，现在支持带参数的函数
FuncDef : BasicType T_ID T_L_PAREN T_R_PAREN Block  {

		// 函数返回类型
		type_attr funcReturnType = $1;

		// 函数名
		var_id_attr funcId = $2;

		// 函数体节点即Block，即$5
		ast_node * blockNode = $5;

		// 形参结点没有，设置为空指针
		ast_node * formalParamsNode = nullptr;

		// 创建函数定义的节点，孩子有类型，函数名，语句块和形参(实际上无)
		// create_func_def函数内会释放funcId中指向的标识符空间，切记，之后不要再释放，之前一定要是通过strdup函数或者malloc分配的空间
		$$ = create_func_def(funcReturnType, funcId, blockNode, formalParamsNode);
	}
	| BasicType T_ID T_L_PAREN FormalParamList T_R_PAREN Block {
		// 函数返回类型
		type_attr funcReturnType = $1;

		// 函数名
		var_id_attr funcId = $2;

		// 函数体节点即Block，即$6
		ast_node * blockNode = $6;

		// 形参节点，即$4
		ast_node * formalParamsNode = $4;

		// 创建函数定义的节点，孩子有类型，函数名，语句块和形参
		$$ = create_func_def(funcReturnType, funcId, blockNode, formalParamsNode);
	}
	;

// 形参列表支持逗号分隔的若干个形参
FormalParamList : FormalParam {
		// 创建形参列表节点，并把当前的形参节点加入
		$$ = create_contain_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS, $1);
	}
	| FormalParamList T_COMMA FormalParam {
		// 左递归增加形参
		$$ = $1->insert_son_node($3);
	}
	;

// 形参定义
FormalParam : BasicType T_ID {
		// 形参类型
		type_attr paramType = $1;

		// 形参名
		var_id_attr paramId = $2;

		// 创建形参节点
		ast_node * type_node = create_type_node(paramType);
		ast_node * id_node = ast_node::New(std::string(paramId.id), paramId.lineno);
		
		// 对于字符型字面量的字符串空间需要释放
		free(paramId.id);
		
		// 创建形参节点，包含类型和名称
		$$ = create_contain_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAM, type_node, id_node);
	}
	| BasicType T_ID ArrayDimensions {
		// 数组形参定义
		type_attr paramType = $1;
		var_id_attr paramId = $2;
		
		// 创建形参节点
		ast_node * type_node = create_type_node(paramType);
		ast_node * id_node = ast_node::New(std::string(paramId.id), paramId.lineno);
		
		// 对于字符型字面量的字符串空间需要释放
		free(paramId.id);
		
		// 创建数组形参节点，包含类型、名称和维度信息
		$$ = create_contain_node(ast_operator_type::AST_OP_FUNC_FORMAL_PARAM_ARRAY, type_node, id_node, $3);
	}
	;

// 语句块的文法Block ： T_L_BRACE BlockItemList? T_R_BRACE
// 其中?代表可有可无，在bison中不支持，需要拆分成两个产生式
// Block ： T_L_BRACE T_R_BRACE | T_L_BRACE BlockItemList T_R_BRACE
Block : T_L_BRACE T_R_BRACE {
		// 语句块没有语句

		// 为了方便创建一个空的Block节点
		$$ = create_contain_node(ast_operator_type::AST_OP_BLOCK);
	}
	| T_L_BRACE BlockItemList T_R_BRACE {
		// 语句块含有语句

		// BlockItemList归约时内部创建Block节点，并把语句加入，这里不创建Block节点
		$$ = $2;
	}
	;

// 语句块内语句列表的文法：BlockItemList : BlockItem+
// Bison不支持正闭包，需修改成左递归形式，便于属性的传递与孩子节点的追加
// 左递归形式的文法为：BlockItemList : BlockItem | BlockItemList BlockItem
BlockItemList : BlockItem {
		// 第一个左侧的孩子节点归约成Block节点，后续语句可持续作为孩子追加到Block节点中
		// 创建一个AST_OP_BLOCK类型的中间节点，孩子为Statement($1)
		$$ = create_contain_node(ast_operator_type::AST_OP_BLOCK, $1);
	}
	| BlockItemList BlockItem {
		// 把BlockItem归约的节点加入到BlockItemList的节点中
		$$ = $1->insert_son_node($2);
	}
	;


// 语句块中子项的文法：BlockItem : Statement
// 目前只支持语句,后续可增加支持变量定义
BlockItem : Statement  {
		// 语句节点传递给归约后的节点上，综合属性
		$$ = $1;
	}
	| VarDecl {
		// 变量声明节点传递给归约后的节点上，综合属性
		$$ = $1;
	}
	;

// 变量声明语句
// 语法：varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON
// 因Bison不支持闭包运算符，因此需要修改成左递归，修改后的文法为：
// VarDecl : VarDeclExpr T_SEMICOLON
// VarDeclExpr: BasicType VarDef | VarDeclExpr T_COMMA varDef
VarDecl : VarDeclExpr T_SEMICOLON {
		$$ = $1;
	}
	;

// 变量声明表达式，可支持逗号分隔定义多个
VarDeclExpr: BasicType VarDef {

		ast_node * type_node = create_type_node($1); // $1 is BasicType
        ast_node * def_result_node = $2;             // $2 is from VarDef
        ast_node * id_or_array_node;
        ast_node * init_expr_node = nullptr;

        if (def_result_node->node_type == ast_operator_type::AST_OP_ASSIGN) {
            // It's an ID with an initializer, packed in a temporary ASSIGN node
            id_or_array_node = def_result_node->sons[0];
            init_expr_node = def_result_node->sons[1];
            def_result_node->sons.clear(); // Detach children before deleting parent
            ast_node::Delete(def_result_node); // Delete the temporary ASSIGN node
        } else {
            // It's a simple ID or an ARRAY_DEF
            id_or_array_node = def_result_node;
        }

        ast_node * decl_node; // This will be AST_OP_VAR_DECL
        if (init_expr_node) {
            decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, id_or_array_node, init_expr_node);
        } else {
            decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, type_node, id_or_array_node);
        }
        decl_node->type = type_node->type; // Set the type on the AST_OP_VAR_DECL node itself
        
        $$ = create_var_decl_stmt_node(decl_node);
        if ($$) { // Ensure create_var_decl_stmt_node returned a node
             ($$)->type = type_node->type; // Explicitly set type for the statement node
        }
	}
	| VarDeclExpr T_COMMA VarDef {

        Type * common_type_for_stmt = $1->type; // Corrected type to Type*

        ast_node * def_result_node = $3; // Result from the current VarDef
        ast_node * id_or_array_node;
        ast_node * init_expr_node = nullptr;

        if (def_result_node->node_type == ast_operator_type::AST_OP_ASSIGN) {
            id_or_array_node = def_result_node->sons[0];
            init_expr_node = def_result_node->sons[1];
            def_result_node->sons.clear();
            ast_node::Delete(def_result_node);
        } else {
            id_or_array_node = def_result_node;
        }

        // Each AST_OP_VAR_DECL needs its own type node instance, created from the common Type*
        ast_node * current_type_node = ast_node::New(common_type_for_stmt); // Should now call ast_node::New(Type*)

        ast_node * decl_node; // This is an AST_OP_VAR_DECL for the current variable
        if (init_expr_node) {
            decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, current_type_node, id_or_array_node, init_expr_node);
        } else {
            decl_node = create_contain_node(ast_operator_type::AST_OP_VAR_DECL, current_type_node, id_or_array_node);
        }
        decl_node->type = current_type_node->type; // Set type for this specific VAR_DECL

        $$ = $1->insert_son_node(decl_node); // Add to existing AST_OP_DECL_STMT
	}
	;

// 变量定义包含变量名，实际上还有初值，这里没有实现。
VarDef : T_ID {
		// 变量ID

		$$ = ast_node::New(var_id_attr{$1.id, $1.lineno});

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);
	}
	| T_ID ArrayDimensions {
		// 数组变量定义
		
		// 创建变量名节点
		ast_node * var_node = ast_node::New(var_id_attr{$1.id, $1.lineno});
		
		// 对于字符型字面量的字符串空间需要释放
		free($1.id);
		
		// 创建数组定义节点，包含变量名和维度信息
		$$ = create_contain_node(ast_operator_type::AST_OP_ARRAY_DEF, var_node, $2);
	}
    | T_ID T_ASSIGN Expr { // New production for initialization
        ast_node * id_node = ast_node::New(var_id_attr{$1.id, $1.lineno});
        free($1.id); // free the strduped id
        // Temporarily use AST_OP_ASSIGN to package id_node and expr_node ($3)
        $$ = create_contain_node(ast_operator_type::AST_OP_ASSIGN, id_node, $3);
    }
	;

// 基本类型，目前只支持整型
BasicType: T_INT { $$ = $1; }
         | T_VOID { $$ = $1; }
         ;

// 语句文法：statement:T_RETURN expr T_SEMICOLON | lVal T_ASSIGN expr T_SEMICOLON
// | block | expr? T_SEMICOLON
// 支持返回语句、赋值语句、语句块、表达式语句
// 其中表达式语句可支持空语句，由于bison不支持?，修改成两条
Statement : T_RETURN Expr T_SEMICOLON {
		// 返回语句

		// 创建返回节点AST_OP_RETURN，其孩子为Expr，即$2
		$$ = create_contain_node(ast_operator_type::AST_OP_RETURN, $2);
	}
	| T_RETURN T_SEMICOLON {
		// 创建一个没有子节点的返回节点 AST_OP_RETURN
		$$ = create_contain_node(ast_operator_type::AST_OP_RETURN);
	}
	| LVal T_ASSIGN Expr T_SEMICOLON {
		// 赋值语句

		// 创建一个AST_OP_ASSIGN类型的中间节点，孩子为LVal($1)和Expr($3)
		$$ = create_contain_node(ast_operator_type::AST_OP_ASSIGN, $1, $3);
	}
	| Block {
		// 语句块

		// 内部已创建block节点，直接传递给Statement
		$$ = $1;
	}
	| Expr T_SEMICOLON {
		// 表达式语句

		// 内部已创建表达式，直接传递给Statement
		$$ = $1;
	}
	| T_SEMICOLON {
		// 空语句

		// 直接返回空指针，需要再把语句加入到语句块时要注意判断，空语句不要加入
		$$ = nullptr;
	}
	| T_IF T_L_PAREN Expr T_R_PAREN Statement {
		// IF语句
		
		// 为条件表达式生成唯一的真假出口标签
		std::string true_label = ".L" + std::to_string(unique_label_counter++);
		std::string false_label = ".L" + std::to_string(unique_label_counter++);
		
		// 设置条件表达式的真假出口标签
		$3->true_label = true_label;  // 真出口，执行语句
		$3->false_label = false_label; // 假出口，跳过语句
		
		// 创建IF语句节点，包含条件表达式和语句
		$$ = create_contain_node(ast_operator_type::AST_OP_IF, $3, $5);
	}
	| T_IF T_L_PAREN Expr T_R_PAREN Statement T_ELSE Statement {
		// IF-ELSE语句
		
		// 为条件表达式生成唯一的真假出口标签
		std::string true_label = ".L" + std::to_string(unique_label_counter++);
		std::string false_label = ".L" + std::to_string(unique_label_counter++);
		std::string end_label = ".L" + std::to_string(unique_label_counter++);
		
		// 设置条件表达式的真假出口标签
		$3->true_label = true_label;  // 真出口，执行IF语句
		$3->false_label = false_label; // 假出口，执行ELSE语句
		
		// 创建IF-ELSE语句节点，包含条件表达式、IF语句和ELSE语句
		$$ = create_contain_node(ast_operator_type::AST_OP_IF_ELSE, $3, $5, $7);
	}
	| T_WHILE T_L_PAREN Expr T_R_PAREN Statement {
		// WHILE循环语句
		
		// 为条件表达式生成唯一的循环入口、循环体和循环结束标签
		std::string loop_start_label_parser_hint = ".L" + std::to_string(unique_label_counter++);
		std::string loop_body_label_parser_hint = ".L" + std::to_string(unique_label_counter++);
		std::string loop_end_label_parser_hint = ".L" + std::to_string(unique_label_counter++);
		
		// 设置条件表达式的真假出口标签 (these are hints for the condition node itself)
		$3->true_label = loop_body_label_parser_hint;
		$3->false_label = loop_end_label_parser_hint;
		
		// 创建WHILE语句节点，包含条件表达式和循环体语句
		ast_node* while_node = create_contain_node(ast_operator_type::AST_OP_WHILE, $3, $5);
		
		$$ = while_node;
	}
	| T_BREAK T_SEMICOLON {
		// BREAK语句
		$$ = create_contain_node(ast_operator_type::AST_OP_BREAK);
	}
	| T_CONTINUE T_SEMICOLON {
		// CONTINUE语句
		$$ = create_contain_node(ast_operator_type::AST_OP_CONTINUE);
	}
	;

// 表达式文法 expr : AddExp
// 表达式目前只支持加法与减法运算
Expr : OrExpr {
		$$ = $1;
	}
	;

// 逻辑或表达式文法：orExpr: andExpr (T_OR andExpr)*
// 由于bison不支持用闭包表达，需要拆分成左递归的形式
OrExpr : AndExpr {
		$$ = $1;
	}
	| AndExpr T_OR AndExpr {
		$$ = create_contain_node(ast_operator_type::AST_OP_OR, $1, $3);
	}
	| OrExpr T_OR AndExpr {
		$$ = create_contain_node(ast_operator_type::AST_OP_OR, $1, $3);
	}
	;

// 逻辑与表达式文法：andExpr: relExpr (T_AND relExpr)*
// 由于bison不支持用闭包表达，需要拆分成左递归的形式
AndExpr : RelExp {
		$$ = $1;
	}
	| RelExp T_AND RelExp {
		$$ = create_contain_node(ast_operator_type::AST_OP_AND, $1, $3);
	}
	| AndExpr T_AND RelExp {
		$$ = create_contain_node(ast_operator_type::AST_OP_AND, $1, $3);
	}
	;

// 关系表达式文法：relExp: addExp (relOp addExp)*
// 由于bison不支持用闭包表达，因此需要拆分成左递归的形式
// 改造后的左递归文法：
// relExp : addExp | addExp relOp addExp | relExp relOp addExp
RelExp : AddExp {
		// 加减表达式
		// 直接传递到归约后的节点
		$$ = $1;
	}
	| AddExp RelOp AddExp {
		// 算术表达式 + 关系运算 + 算术表达式
		// 创建关系运算节点，孩子为两个算术表达式节点
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	| RelExp RelOp AddExp {
		// 关系表达式 + 关系运算 + 算术表达式
		// 创建关系运算节点，孩子为关系表达式和算术表达式
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	;

// 关系运算符
RelOp: T_GT {
		$$ = (int)ast_operator_type::AST_OP_GT;
	}
	| T_GE {
		$$ = (int)ast_operator_type::AST_OP_GE;
	}
	| T_LT {
		$$ = (int)ast_operator_type::AST_OP_LT;
	}
	| T_LE {
		$$ = (int)ast_operator_type::AST_OP_LE;
	}
	| T_EQ {
		$$ = (int)ast_operator_type::AST_OP_EQ;
	}
	| T_NE {
		$$ = (int)ast_operator_type::AST_OP_NE;
	}
	;

// 加减表达式文法：addExp: unaryExp (addOp unaryExp)*
// 由于bison不支持用闭包表达，因此需要拆分成左递归的形式
// 改造后的左递归文法：
// addExp : unaryExp | unaryExp addOp unaryExp | addExp addOp unaryExp
AddExp : MulExp {
		// 乘除模表达式
		// 直接传递到归约后的节点
		$$ = $1;
	}
	| MulExp AddOp MulExp {
		// 两个乘除模表达式的加减运算
		// 创建加减运算节点，其孩子为两个乘除模表达式节点
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	| AddExp AddOp MulExp {
		// 左递归形式可通过加减连接多个乘除模表达式
		// 创建加减运算节点，孩子为AddExp($1)和MulExp($3)
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	;

// 加减运算符
AddOp: T_ADD {
		$$ = (int)ast_operator_type::AST_OP_ADD;
	}
	| T_SUB {
		$$ = (int)ast_operator_type::AST_OP_SUB;
	}
	;

// 乘除模表达式文法：mulExp: unaryExp (mulOp unaryExp)*
// 由于bison不支持用闭包表达，因此需要拆分成左递归的形式
// 改造后的左递归文法：
// mulExp : unaryExp | unaryExp mulOp unaryExp | mulExp mulOp unaryExp
MulExp : UnaryExp {
		// 一元表达式
		// 直接传递到归约后的节点
		$$ = $1;
	}
	| UnaryExp MulOp UnaryExp {
		// 两个一元表达式的乘除模运算
		// 创建乘除模运算节点，其孩子为两个一元表达式节点
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	| MulExp MulOp UnaryExp {
		// 左递归形式可通过乘除模连接多个一元表达式
		// 创建乘除模运算节点，孩子为MulExp($1)和UnaryExp($3)
		$$ = create_contain_node(ast_operator_type($2), $1, $3);
	}
	;

// 乘除模运算符
MulOp: T_MUL {
		$$ = (int)ast_operator_type::AST_OP_MUL;
	}
	| T_DIV {
		$$ = (int)ast_operator_type::AST_OP_DIV;
	}
	| T_MOD {
		$$ = (int)ast_operator_type::AST_OP_MOD;
	}
	;

// 目前一元表达式可以为基本表达式、函数调用，其中函数调用的实参可有可无
// 其文法为：unaryExp: primaryExp | T_ID T_L_PAREN realParamList? T_R_PAREN
// 由于bison不支持？表达，因此变更后的文法为：
// unaryExp: primaryExp | T_ID T_L_PAREN T_R_PAREN | T_ID T_L_PAREN realParamList T_R_PAREN
// 新增：支持逻辑非运算 T_NOT unaryExp
UnaryExp : PrimaryExp {
		// 基本表达式

		// 传递到归约后的UnaryExp上
		$$ = $1;
	}
	| T_SUB UnaryExp {
		// 求负运算
		
		// 创建求负运算节点
		$$ = create_contain_node(ast_operator_type::AST_OP_NEG, $2);
	}
	| T_NOT UnaryExp {
		// 逻辑非运算
		
		// 创建逻辑非运算节点
		$$ = create_contain_node(ast_operator_type::AST_OP_NOT, $2);
	}
	| T_ID T_L_PAREN T_R_PAREN {
		// 没有实参的函数调用

		// 创建函数调用名终结符节点
		ast_node * name_node = ast_node::New(std::string($1.id), $1.lineno);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);

		// 实参列表
		ast_node * paramListNode = nullptr;

		// 创建函数调用节点，其孩子为被调用函数名和实参，实参为空，但函数内部会创建实参列表节点，无孩子
		$$ = create_func_call(name_node, paramListNode);

	}
	| T_ID T_L_PAREN RealParamList T_R_PAREN {
		// 含有实参的函数调用

		// 创建函数调用名终结符节点
		ast_node * name_node = ast_node::New(std::string($1.id), $1.lineno);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);

		// 实参列表
		ast_node * paramListNode = $3;

		// 创建函数调用节点，其孩子为被调用函数名和实参，实参不为空
		$$ = create_func_call(name_node, paramListNode);
	}
	;

// 基本表达式支持无符号整型字面量、带括号的表达式、具有左值属性的表达式
// 其文法为：primaryExp: T_L_PAREN expr T_R_PAREN | T_DIGIT | lVal
PrimaryExp :  T_L_PAREN Expr T_R_PAREN {
		// 带有括号的表达式
		$$ = $2;
	}
	| T_DIGIT {
        	// 无符号整型字面量

		// 创建一个无符号整型的终结符节点
		$$ = ast_node::New($1);
	}
	| LVal  {
		// 具有左值的表达式

		// 直接传递到归约后的非终结符号PrimaryExp
		$$ = $1;
	}
	;

// 实参表达式支持逗号分隔的若干个表达式
// 其文法为：realParamList: expr (T_COMMA expr)*
// 由于Bison不支持闭包运算符表达，修改成左递归形式的文法
// 左递归文法为：RealParamList : Expr | 左递归文法为：RealParamList T_COMMA expr
RealParamList : Expr {
		// 创建实参列表节点，并把当前的Expr节点加入
		$$ = create_contain_node(ast_operator_type::AST_OP_FUNC_REAL_PARAMS, $1);
	}
	| RealParamList T_COMMA Expr {
		// 左递归增加实参表达式
		$$ = $1->insert_son_node($3);
	}
	;

// 左值表达式，目前只支持变量名，实际上还有下标变量
LVal : T_ID {
		// 变量名终结符

		// 创建变量名终结符节点
		$$ = ast_node::New($1);

		// 对于字符型字面量的字符串空间需要释放，因词法用到了strdup进行了字符串复制
		free($1.id);
	}
	| ArrayAccess {
		// 数组访问表达式
		$$ = $1;
	}
	;

// 数组维度定义，支持多维数组
ArrayDimensions : T_L_BRACKET T_DIGIT T_R_BRACKET {
		// 一维数组
		ast_node * dim_node = ast_node::New($2);
		$$ = create_contain_node(ast_operator_type::AST_OP_ARRAY_DIMENSIONS, dim_node);
	}
	| T_L_BRACKET T_R_BRACKET {
		// 空维度数组（用于函数形参）
		ast_node * dim_node = ast_node::New(digit_int_attr{0, yylineno}); // 使用0表示空维度
		$$ = create_contain_node(ast_operator_type::AST_OP_ARRAY_DIMENSIONS, dim_node);
	}
	| ArrayDimensions T_L_BRACKET T_DIGIT T_R_BRACKET {
		// 多维数组，左递归添加维度
		ast_node * dim_node = ast_node::New($3);
		$$ = $1->insert_son_node(dim_node);
	}
	| ArrayDimensions T_L_BRACKET T_R_BRACKET {
		// 多维数组的空维度
		ast_node * dim_node = ast_node::New(digit_int_attr{0, yylineno}); // 使用0表示空维度
		$$ = $1->insert_son_node(dim_node);
	}
	;

// 数组访问表达式
ArrayAccess : T_ID T_L_BRACKET Expr T_R_BRACKET {
		// 一维数组访问
		ast_node * var_node = ast_node::New($1);
		free($1.id);
		
		// 创建数组访问节点，包含数组名和索引表达式
		$$ = create_contain_node(ast_operator_type::AST_OP_ARRAY_ACCESS, var_node, $3);
	}
	| ArrayAccess T_L_BRACKET Expr T_R_BRACKET {
		// 多维数组访问，左递归添加索引
		$$ = $1->insert_son_node($3);
	}
	;

%%

// 语法识别错误要调用函数的定义
void yyerror(char * msg)
{
    printf("Line %d: %s\n", yylineno, msg);
}
