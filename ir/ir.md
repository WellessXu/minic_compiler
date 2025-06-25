表达式运算指令
p表达式运算结果赋值给临时变量，一般不直接赋值给局部变量或全局变量
p表达式右侧的源操作数变量可以是可右值操作的所有变量，包含常量
p 关系运算
p 格式：变量1 = cmp 比较运算符 变量2，变量3
 p le: 小于等于    lt：小于   gt：大于    ge：大于等于   ne：不等于   eq：等于
p 说明：比较结果（变量1）要求为bool类型，即i1。

p表达式运算指令
p表达式运算结果赋值给临时变量，一般不直接赋值给局部变量或全局变量
p表达式右侧的源操作数变量可以是可右值操作的所有变量，包含常量
p 逻辑运算
p 在语义分析的过程中，逻辑运算采用短路求值，转换为有条件或者无条件指令实现。因
此中间IR不定义逻辑运算指令。

跳转与标签指令
p Label指令
p 作用：用于指令的跳转目标，类似与C语言的label语句
p 格式：标识符
p 说明：标识符用于定义Label名称，以.L开头的标识符。要求函数内唯一，不可重复。
p 无条件指令
p 格式：br label 标识符
p 说明：无条件跳转到标识符所代表的位置，标识符是Label指令所定义的Label名，其中
关键字label可省略。
p 注意：Label名可在本条指令的前面，也可在本条指令的后面。

跳转与标签指令
p 有条件指令
p 格式1：bc condvar, label X, label Y
 p 说明：condvar条件临时变量，类型为i1； X为真跳转标签，Y假跳转标签；本指令表示若
condvar为真，则跳转到X处执行，否则跳转到Y处执行。
p 格式2： bt condvar, label X
 p 说明： condvar条件临时变量，类型为i1； X为真跳转标签；本指令表示若condvar为真，则跳转
到X处执行，否则顺序执行下一条指令。
p 格式3： bf condvar, label X 
p 说明： condvar条件临时变量，类型为i1； X为假跳转Label名；本指令表示若condvar为假，则
跳转到X处执行，否则顺序执行下一条指令。
p 建议使用bc指令，不要使用bt或者bf指令

例子
int g1;
 int main ()
 {
    int a;
    int b;
    int c;
    g1 = 1;
    a = getint();
    b = getint();
    c = getint();
    if((a < b) && (a < c)) {
        g1 = 0;
    }
    putint(g1);
    return 0;
 }

  declare i32 @g1
 define i32 @main() {
 declare i32 %l0
 declare i32 %l1 ; variable: a
 declare i32 %l2 ; variable: b
 declare i32 %l3 ; variable: c
 declare i32 %t4
 declare i32 %t5
 declare i32 %t6
 declare i1 %t7
 declare i1 %t8
 entry
 @g1 = 1
 %t4 = call i32 @getint()
 %l1 = %t4
 %t5 = call i32 @getint()
 %l2 = %t5
 %t6 = call i32 @getint()
 %l3 = %t6
 %t7 = cmp lt %l1, %l2          ; a < b
 bc %t7, label .L2, label .L4    ; 逻辑与短路求值
.L2:
 %t8 = cmp lt %l1, %l3          ; a < c
 bc %t8, label .L3, label .L4
 .L3:                                                   ; 真出口
@g1 = 0
 br label .L4
 .L4:                                                    ; 假出口和if语句的出口
call void @putint(i32 @g1)
 %l0 = 0
 exit %l0
 }

int g1;
 int main()
 {
    int a = 1;
    int sum = 0;
    while(a < 101) {
        sum = sum + a;
        a = a + 1;
    }
    putint(sum);
    return 0;
 }

  declare i32 @g1
 define i32 @main() {
 declare i32 %l0
 declare i32 %l1 ; variable: a
 declare i32 %l2 ; variable: sum
 declare i1 %t3
 declare i32 %t4
 declare i32 %t5
 entry
 %l1 = 1
 %l2 = 0
 br label .L2                         ; 这个跳转指令可省略
.L2:                                                        ; while循环的入口
%t3 = cmp lt %l1, 101
 bc %t3, label .L3, label .L4
 .L3:                                                          ; while循环体入口
%t4 = add %l2, %l1
 %l2 = %t4
 %t5 = add %l1, 1
 %l1 = %t5
 br label .L2                         ; 跳转到循环的入口 
.L4:                                                        ;  while循环的出口
call void @putint(i32 %l2)
 %l0 = 0
 exit %l0
 }
