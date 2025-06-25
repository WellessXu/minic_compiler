# MiniC编译器实现

本文档详细说明如何在MiniC编译器中实现乘法(`*`)、除法(`/`)和求余(`%`)运算，以及关系表达式支持，涵盖三种词法语法分析技术和IR代码生成。

## 目录
1. [概述](#概述)
2. [算术运算实现](#算术运算实现)
   - [FlexBison实现](#算术运算-flexbison实现)
   - [Antlr4实现](#算术运算-antlr4实现)
   - [递归下降解析实现](#算术运算-递归下降解析实现)
   - [IR生成](#算术运算-ir生成)
3. [关系表达式实现](#关系表达式实现)
   - [关系运算符定义](#关系运算符定义)
   - [FlexBison实现](#关系表达式-flexbison实现)
   - [Antlr4实现](#关系表达式-antlr4实现)
   - [递归下降解析实现](#关系表达式-递归下降解析实现)
   - [IR生成](#关系表达式-ir生成)
4. [测试与验证](#测试与验证)

## 概述

在编译器中添加新的功能需要修改多个组件：
- **词法分析器**：识别新的运算符符号
- **语法分析器**：定义新运算符的语法规则和优先级
- **AST构建**：为新运算符创建AST节点
- **IR生成**：为新运算符生成相应的IR指令

本文档将详细说明如何实现算术运算和关系表达式，以丰富MiniC语言的功能。

## 算术运算实现

### 算术运算-FlexBison实现

#### 1. 词法分析器修改 (MiniC.l)

在Flex词法分析器中添加新的token定义：

```c
"*"         { return T_MUL; }
"/"         { return T_DIV; }
"%"         { return T_MOD; }
```

#### 2. 语法分析器修改 (MiniC.y)

##### 2.1 声明新token

```c
// 运算符
%token T_ASSIGN T_SUB T_ADD T_MUL T_DIV T_MOD
```

##### 2.2 添加非终结符

```c
%type <node> MulExp
%type <op_class> MulOp
```

##### 2.3 定义新的语法规则

修改表达式语法结构，引入乘除模表达式作为加减表达式的组成部分，实现运算符优先级：

```c
// 表达式文法 expr : AddExp
Expr : AddExp {
    // 直接传递给归约后的节点
    $$ = $1;
}
;

// 加减表达式文法：addExp: mulExp (addOp mulExp)*
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
```

#### 3. 抽象语法树修改 (AST.h)

在AST.h中添加新的运算符类型：

```cpp
enum class ast_operator_type : std::int16_t {
    // 已有的运算符...
    AST_OP_ADD,
    AST_OP_SUB,
    AST_OP_MUL,  // 乘法
    AST_OP_DIV,  // 除法
    AST_OP_MOD,  // 求余
    // 其他操作符...
};
```

### 算术运算-Antlr4实现

#### 1. 语法定义修改 (MiniC.g4)

在Antlr4 G4文件中，添加乘法、除法和求余运算的词法和语法规则：

##### 1.1 词法规则

```antlr
MUL: '*';
DIV: '/';
MOD: '%';
```

##### 1.2 语法规则

修改表达式相关的语法规则，确保乘除模运算优先级高于加减：

```antlr
expr
    : addExp
    ;

addExp
    : mulExp                               # MulExpression
    | addExp ('+' | '-') mulExp           # AddSubExpression
    ;

mulExp
    : unaryExp                             # UnaryExpression
    | mulExp ('*' | '/' | '%') unaryExp   # MulDivModExpression
    ;

unaryExp
    : primaryExp                           # PrimaryExpression
    | '-' unaryExp                         # NegateExpression
    // 其他一元表达式规则...
    ;
```

#### 2. 访问者实现 (Antlr4CSTVisitor.cpp/h)

##### 2.1 添加访问方法声明 (Antlr4CSTVisitor.h)

```cpp
// 在类声明中添加新的访问方法
virtual antlrcpp::Any visitMulDivModExpression(MiniCParser::MulDivModExpressionContext *ctx) override;
```

##### 2.2 实现访问方法 (Antlr4CSTVisitor.cpp)

```cpp
antlrcpp::Any Antlr4CSTVisitor::visitMulDivModExpression(MiniCParser::MulDivModExpressionContext *ctx) {
    // 获取左操作数
    ast_node* left = visit(ctx->mulExp());
    
    // 获取右操作数
    ast_node* right = visit(ctx->unaryExp());
    
    // 确定操作类型
    ast_operator_type op_type;
    if (ctx->MUL()) {
        op_type = ast_operator_type::AST_OP_MUL;
    } else if (ctx->DIV()) {
        op_type = ast_operator_type::AST_OP_DIV;
    } else if (ctx->MOD()) {
        op_type = ast_operator_type::AST_OP_MOD;
    } else {
        // 这不应该发生，但为了安全
        return nullptr;
    }
    
    // 创建并返回操作节点
    return create_contain_node(op_type, left, right);
}
```

### 算术运算-递归下降解析实现

#### 1. 词法分析器修改

确保词法分析器可以识别新的运算符（通常与FlexBison使用相同的词法分析器）。

#### 2. 递归下降解析器修改 (RecursiveDescentParser.cpp)

##### 2.1 添加乘除运算符函数

```cpp
// 识别乘法、除法和求余运算符
static ast_operator_type mulOp()
{
    if (match(T_MUL)) {
        return ast_operator_type::AST_OP_MUL;
    } else if (match(T_DIV)) {
        return ast_operator_type::AST_OP_DIV;
    } else if (match(T_MOD)) {
        return ast_operator_type::AST_OP_MOD;
    } else {
        return ast_operator_type::AST_OP_NONE;
    }
}
```

##### 2.2 实现乘除模表达式分析函数

```cpp
// 乘除模表达式的递归下降分析：mulExp -> unaryExp (mulOp unaryExp)*
static ast_node* mulExp()
{
    // 解析第一个一元表达式
    ast_node* node = unaryExp();
    
    // 循环查看是否有乘法、除法或求余运算符
    for (;;) {
        ast_operator_type op = mulOp();
        if (op == ast_operator_type::AST_OP_NONE) {
            break; // 不是乘除模运算符，退出循环
        }
        
        // 解析右侧操作数
        ast_node* right = unaryExp();
        
        // 创建二元运算节点
        node = create_contain_node(op, node, right);
    }
    
    return node;
}
```

##### 2.3 修改加减表达式分析函数

```cpp
// 加减表达式的递归下降分析：addExp -> mulExp (addOp mulExp)*
static ast_node* addExp()
{
    // 解析乘除模表达式作为左操作数
    ast_node* node = mulExp();
    
    // 循环查看是否有加减运算符
    for (;;) {
        ast_operator_type op = addOp();
        if (op == ast_operator_type::AST_OP_NONE) {
            break; // 不是加减运算符，退出循环
        }
        
        // 解析右侧的乘除模表达式
        ast_node* right = mulExp();
        
        // 创建二元运算节点
        node = create_contain_node(op, node, right);
    }
    
    return node;
}
```

### 算术运算-IR生成

在IRGenerator中添加对新运算符的处理：

```cpp
// 二元运算处理函数
Value* IRGenerator::ir_binary_operation(ast_node* node)
{
    // 递归生成左右操作数的IR
    Value* left_val = generate(node->sons[0]);
    Value* right_val = generate(node->sons[1]);
    
    if (!left_val || !right_val) {
        return nullptr;
    }
    
    // 根据运算符类型生成相应IR指令
    switch (node->node_type) {
        case ast_operator_type::AST_OP_ADD:
            return builder.CreateAdd(left_val, right_val, "addtmp");
            
        case ast_operator_type::AST_OP_SUB:
            return builder.CreateSub(left_val, right_val, "subtmp");
            
        case ast_operator_type::AST_OP_MUL:
            return builder.CreateMul(left_val, right_val, "multmp");
            
        case ast_operator_type::AST_OP_DIV:
            // 注意：这里假设我们处理的都是有符号整数
            return builder.CreateSDiv(left_val, right_val, "divtmp");
            
        case ast_operator_type::AST_OP_MOD:
            return builder.CreateSRem(left_val, right_val, "modtmp");
            
        default:
            // 未知的二元运算符
            return nullptr;
    }
}
```

## 关系表达式实现

关系表达式是编程语言中的重要组成部分，用于比较两个值的大小关系。关系表达式包括大于、小于、大于等于、小于等于、等于和不等于等运算符。

### 关系运算符定义

在AST.h中添加关系运算符类型：

```cpp
enum class ast_operator_type : std::int16_t {
    // 已有的运算符...
    AST_OP_GT,    // 大于 >
    AST_OP_GE,    // 大于等于 >=
    AST_OP_LT,    // 小于 <
    AST_OP_LE,    // 小于等于 <=
    AST_OP_EQ,    // 等于 ==
    AST_OP_NE,    // 不等于 !=
    // 其他操作符...
};
```

### 关系表达式-FlexBison实现

#### 1. 词法分析器修改 (MiniC.l)

```c
">"         { return T_GT; }
">="        { return T_GE; }
"<"         { return T_LT; }
"<="        { return T_LE; }
"=="        { return T_EQ; }
"!="        { return T_NE; }
```

#### 2. 语法分析器修改 (MiniC.y)

```c
// 声明token
%token T_GT T_GE T_LT T_LE T_EQ T_NE

// 声明非终结符
%type <node> RelExp
%type <op_class> RelOp

// 修改Expr规则，使其使用关系表达式
Expr : RelExp {
    $$ = $1;
}
;

// 关系表达式文法：relExp: addExp (relOp addExp)?
RelExp : AddExp {
    $$ = $1;
}
| AddExp RelOp AddExp {
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
```

### 关系表达式-Antlr4实现

#### 1. 词法规则 (MiniC.g4)

```antlr
GT: '>';
GE: '>=';
LT: '<';
LE: '<=';
EQ: '==';
NE: '!=';
```

#### 2. 语法规则 (MiniC.g4)

```antlr
// 修改表达式规则
expr
    : relExp
    ;

relExp
    : addExp                               # AddExpression
    | addExp relOp addExp                  # RelExpression
    ;

relOp
    : '>'                                  # GT
    | '>='                                 # GE
    | '<'                                  # LT
    | '<='                                 # LE
    | '=='                                 # EQ
    | '!='                                 # NE
    ;
```

#### 3. 访问者实现

##### 3.1 添加访问方法声明 (Antlr4CSTVisitor.h)

```cpp
virtual antlrcpp::Any visitRelExpression(MiniCParser::RelExpressionContext *ctx) override;
```

##### 3.2 实现访问方法 (Antlr4CSTVisitor.cpp)

```cpp
antlrcpp::Any Antlr4CSTVisitor::visitRelExpression(MiniCParser::RelExpressionContext *ctx) {
    // 获取左右操作数
    ast_node* left = visit(ctx->addExp(0));
    ast_node* right = visit(ctx->addExp(1));
    
    // 确定操作类型
    ast_operator_type op_type;
    if (ctx->relOp()->GT()) {
        op_type = ast_operator_type::AST_OP_GT;
    } else if (ctx->relOp()->GE()) {
        op_type = ast_operator_type::AST_OP_GE;
    } else if (ctx->relOp()->LT()) {
        op_type = ast_operator_type::AST_OP_LT;
    } else if (ctx->relOp()->LE()) {
        op_type = ast_operator_type::AST_OP_LE;
    } else if (ctx->relOp()->EQ()) {
        op_type = ast_operator_type::AST_OP_EQ;
    } else if (ctx->relOp()->NE()) {
        op_type = ast_operator_type::AST_OP_NE;
    } else {
        // 这不应该发生，但为了安全
        return nullptr;
    }
    
    // 创建并返回关系运算节点
    return create_contain_node(op_type, left, right);
}
```

### 关系表达式-递归下降解析实现

#### 1. 添加关系运算符函数

```cpp
// 识别关系运算符
static ast_operator_type relOp()
{
    if (match(T_GT)) {
        return ast_operator_type::AST_OP_GT;
    } else if (match(T_GE)) {
        return ast_operator_type::AST_OP_GE;
    } else if (match(T_LT)) {
        return ast_operator_type::AST_OP_LT;
    } else if (match(T_LE)) {
        return ast_operator_type::AST_OP_LE;
    } else if (match(T_EQ)) {
        return ast_operator_type::AST_OP_EQ;
    } else if (match(T_NE)) {
        return ast_operator_type::AST_OP_NE;
    } else {
        return ast_operator_type::AST_OP_NONE;
    }
}
```

#### 2. 实现关系表达式分析函数

```cpp
// 关系表达式的递归下降分析：relExp -> addExp (relOp addExp)?
static ast_node* relExp()
{
    // 解析加减表达式作为左操作数
    ast_node* node = addExp();
    
    // 查看是否有关系运算符
    ast_operator_type op = relOp();
    if (op == ast_operator_type::AST_OP_NONE) {
        return node; // 没有关系运算符，直接返回加减表达式
    }
    
    // 解析右侧的加减表达式
    ast_node* right = addExp();
    
    // 创建关系运算节点
    return create_contain_node(op, node, right);
}
```

#### 3. 修改表达式分析函数

```cpp
// 表达式的递归下降分析：expr -> relExp
static ast_node* expr()
{
    // 解析关系表达式
    return relExp();
}
```

### 关系表达式-IR生成

在IRGenerator中添加关系表达式的处理，关系表达式需要考虑真假出口标签：

```cpp
// 关系表达式处理函数
Value* IRGenerator::ir_rel_operation(ast_node* node)
{
    // 递归生成左右操作数的IR
    Value* left_val = generate(node->sons[0]);
    Value* right_val = generate(node->sons[1]);
    
    if (!left_val || !right_val) {
        return nullptr;
    }
    
    // 创建比较指令
    Value* cmp_result = nullptr;
    
    // 处理可能的类型转换
    Type* left_type = left_val->getType();
    Type* right_type = right_val->getType();
    
    // 如果类型不匹配，进行必要的类型转换
    if (left_type != right_type) {
        // 这里简化处理，假设只需要处理整数类型
        if (left_type->isIntegerTy() && right_type->isIntegerTy()) {
            // 将位数较小的整数类型转换为位数较大的类型
            if (left_type->getIntegerBitWidth() < right_type->getIntegerBitWidth()) {
                left_val = builder.CreateIntCast(left_val, right_type, true, "cast");
            } else {
                right_val = builder.CreateIntCast(right_val, left_type, true, "cast");
            }
        }
    }
    
    // 根据运算符类型生成相应比较指令
    switch (node->node_type) {
        case ast_operator_type::AST_OP_GT:
            cmp_result = builder.CreateICmpSGT(left_val, right_val, "gttmp");
            break;
        case ast_operator_type::AST_OP_GE:
            cmp_result = builder.CreateICmpSGE(left_val, right_val, "getmp");
            break;
        case ast_operator_type::AST_OP_LT:
            cmp_result = builder.CreateICmpSLT(left_val, right_val, "lttmp");
            break;
        case ast_operator_type::AST_OP_LE:
            cmp_result = builder.CreateICmpSLE(left_val, right_val, "letmp");
            break;
        case ast_operator_type::AST_OP_EQ:
            cmp_result = builder.CreateICmpEQ(left_val, right_val, "eqtmp");
            break;
        case ast_operator_type::AST_OP_NE:
            cmp_result = builder.CreateICmpNE(left_val, right_val, "netmp");
            break;
        default:
            return nullptr;
    }
    
    // 如果父节点已设置真假出口标签，则生成条件跳转指令
    if (!node->true_label.empty() && !node->false_label.empty()) {
        // 获取或创建目标基本块
        BasicBlock* true_bb = getOrCreateBasicBlock(node->true_label);
        BasicBlock* false_bb = getOrCreateBasicBlock(node->false_label);
        
        // 创建条件分支指令
        builder.CreateCondBr(cmp_result, true_bb, false_bb);
    }
    
    return cmp_result;
}

// 辅助函数：获取或创建基本块
BasicBlock* IRGenerator::getOrCreateBasicBlock(const std::string& name)
{
    // 查找是否已存在同名基本块
    Function* func = builder.GetInsertBlock()->getParent();
    for (auto& bb : *func) {
        if (bb.getName() == name) {
            return &bb;
        }
    }
    
    // 不存在则创建新的基本块
    return BasicBlock::Create(context, name, func);
}
```

## 测试与验证

### 算术运算测试

创建以下测试用例验证算术运算符的功能：

```c
int main() {
    int a = 10;
    int b = 3;
    int c;
    
    // 测试乘法
    c = a * b;       // 应为30
    
    // 测试除法
    c = a / b;       // 应为3
    
    // 测试求余
    c = a % b;       // 应为1
    
    // 测试混合运算
    c = (a + b) * 2; // 应为26
    c = a * b + 5;   // 应为35
    c = a * (b + 1); // 应为40
    
    return c;
}
```

### 关系表达式测试

创建以下测试用例验证关系表达式的功能：

```c
int main() {
    int a = 10;
    int b = 5;
    int result = 0;
    
    // 测试各种关系表达式
    if (a > b) {
        result = 1;  // 应执行
    }
    
    if (a >= b) {
        result = 2;  // 应执行
    }
    
    if (a < b) {
        result = 3;  // 不应执行
    }
    
    if (a <= b) {
        result = 4;  // 不应执行
    }
    
    if (a == b) {
        result = 5;  // 不应执行
    }
    
    if (a != b) {
        result = 6;  // 应执行
    }
    
    // 测试复合条件
    if (a > b && a != 5) {
        result = 7;  // 应执行
    }
    
    return result;
}
```

生成的IR示例（以`a < b`为例）：

```llvm
; 计算a < b
%t1 = load i32, i32* %a
%t2 = load i32, i32* %b
%cmp = icmp slt i32 %t1, %t2
br i1 %cmp, label %then_label, label %else_label

then_label:
    ; 真分支代码
    store i32 3, i32* %result
    br label %endif_label

else_label:
    ; 假分支代码
    br label %endif_label

endif_label:
    ; 后续代码
```

## 总结

通过本文档的修改，MiniC编译器已成功实现了:

1. **算术运算**：
   - 添加乘法、除法和求余运算支持
   - 实现正确的运算符优先级处理

2. **关系表达式**：
   - 添加六种关系运算符（>、>=、<、<=、==、!=）
   - 实现类型转换和条件跳转生成
   - 处理真假出口标签

三种前端技术（FlexBison、Antlr4和递归下降）各有特点，但都能有效实现这些功能。通过这些修改，MiniC语言的表达能力得到了显著增强，能够处理更复杂的算术和逻辑表达式。
