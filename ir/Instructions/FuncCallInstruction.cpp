///
/// @file FuncCallInstruction.cpp
/// @brief 函数调用指令
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "FuncCallInstruction.h"
#include "Function.h"
#include "Common.h"
#include "Type.h"
#include "Types/ArrayType.h"
#include "Types/PointerType.h"

/// @brief 含有参数的函数调用
/// @param srcVal 函数的实参Value
/// @param result 保存返回值的Value
FuncCallInstruction::FuncCallInstruction(Function * _func,
                                         Function * calledFunc,
                                         std::vector<Value *> & _srcVal,
                                         Type * _type)
    : Instruction(_func, IRInstOperator::IRINST_OP_FUNC_CALL, _type), calledFunction(calledFunc)
{
    name = calledFunc->getName();

    // 实参拷贝
    for (auto & val: _srcVal) {
        addOperand(val);
    }
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void FuncCallInstruction::toString(std::string & str)
{
    int32_t argCount = func->getRealArgcount();
    int32_t operandsNum = getOperandsNum();

    if (operandsNum != argCount) {
        // 两者不一致 也可能没有ARG指令，正常
        if (argCount != 0) {
            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    // TODO 这里应该根据函数名查找函数定义或者声明获取函数的类型
    // 这里假定所有函数返回类型要么是i32，要么是void
    // 函数参数的类型是i32

    if (type->isVoidType()) {
        // 函数没有返回值设置
        str = "call void " + calledFunction->getIRName() + "(";
    } else {
        // 函数有返回值要设置到结果变量中
        str = getIRName() + " = call " + type->toString() + " " + calledFunction->getIRName() + "(";
    }

    // 检查被调用函数声明的参数数量与实际提供的参数数量是否匹配
    int32_t declaredParamCount = calledFunction->getParams().size();
    
    // 只有当函数声明有参数且提供了实参时才输出参数
    if (declaredParamCount > 0 && operandsNum > 0) {
        if (argCount == 0) {  // 如果没有ARG指令直接输出实参
            for (int32_t k = 0; k < std::min(operandsNum, declaredParamCount); ++k) {
                auto operand = getOperand(k);
                std::string argStr;
                if (operand->getType()->isArrayType()) {
                    ArrayType* arrayType = static_cast<ArrayType*>(operand->getType());
                    // 输出格式：元素类型 + 空格 + 操作数名 + 维度 (例如 i32 @a[2])
                    argStr = arrayType->getElementType()->toString() + " " + operand->getIRName();
                    const auto& dims = arrayType->getDimensions();
                    for (int32_t dim : dims) {
                        argStr += "[" + std::to_string(dim) + "]";
                    }
                } else if (operand->getType()->isPointerType() && calledFunction->getParams()[k]->getType()->isPointerType()) {
                    // 如果实参是指针，且对应形参也是指针（代表数组传递的情况）
                    // 假定我们传递的是数组首地址，并且希望以 数组名[维度] 的形式打印（通常是 数组名[0] 或 数组名[实际大小]）
                    // 对于 call @test(i32 @a[2]) 这样的情况，@a 本身是一个全局数组，其类型已经是 ArrayType
                    // 如果 @a 是一个指针，那么其类型是 PointerType，这里可能需要根据具体情况调整
                    // 这里我们假设如果源操作数是ArrayType，则已在上面的if分支中处理
                    // 如果源操作数是PointerType，我们可能需要知道数组的大小，这通常在ArrayType中
                    // 因此，一个更鲁棒的做法是在生成IR时，如果传递的是数组，确保操作数本身就是ArrayType
                    // 或者，如果它是一个指针，那么调用点需要知道它代表的数组的维度信息
                    // 为简化，这里我们先假定作为数组参数传递的指针，其 IRName 已经包含了必要的表示，或者上层已经转换
                    // 或者，我们从对应的形参获取信息，但形参可能只知道是 i32 %[name][0]
                    // 最安全的还是让操作数类型能够反映它是如何被使用的。
                    // 修正：如果操作数是数组（ArrayType），它会包含维度。全局数组@a就是ArrayType。
                    // 如果是局部指针变量传给数组形参，它会是PointerType。
                    // 正确的逻辑应该是如果操作数是 ArrayType，就使用其维度。
                    // 如果操作数是 PointerType 但对应的形参是数组形式(PointerType)，则打印为 i32 %ptr (不加[0])
                    // 因为call指令的参数类型和名字就够了, 类似 call i32 @test(i32 %ptr_param)
                    // LLVM IR 通常是 `type value`，例如 `i32* %ptr` 或 `[2 x i32]* %arr_ptr`
                    // 我们之前在 Function.cpp 中将形参打印为 i32 %name[0]
                    // 为了匹配，如果实参是数组，就打印 i32 @name[size]
                    // 如果实参是指针（指向数组），并且形参是 i32 name[0] 这种形式，那么实参打印为 i32 %ptr_name (不带下标)
                    PointerType* ptrType = static_cast<PointerType*>(operand->getType());
                    argStr = ptrType->getPointeeType()->toString() + " " + operand->getIRName(); 

                } else {
                    argStr = operand->getType()->toString() + " " + operand->getIRName();
                }
                str += argStr;
                if (k != (std::min(operandsNum, declaredParamCount) - 1)) {
                    str += ", ";
                }
            }
        }
    }

    str += ")";

    // 要清零
    func->realArgCountReset();
}

///
/// @brief 获取被调用函数的名字
/// @return std::string 被调用函数名字
///
std::string FuncCallInstruction::getCalledName() const
{
    return calledFunction->getName();
}
