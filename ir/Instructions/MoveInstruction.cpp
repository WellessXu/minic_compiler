///
/// @file MoveInstruction.cpp
/// @brief Move指令，也就是DragonIR的Asssign指令
///
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

#include "VoidType.h"
#include "PointerType.h"

#include "MoveInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType())
{
    addOperand(_result);
    addOperand(_srcVal1);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void MoveInstruction::toString(std::string & str)
{
    Value *dstVal = getOperand(0), *srcVal = getOperand(1);
    bool dstIsPtr = dstVal->getType()->isPointerType();
    bool srcIsPtr = srcVal->getType()->isPointerType();

    if (dstIsPtr && !srcIsPtr) {
        // 目标是指针，源不是指针: *目标 = 源 (类似Store)
        str = "*" + dstVal->getIRName() + " = " + srcVal->getIRName();
    } else if (!dstIsPtr && srcIsPtr) {
        // 目标不是指针，源是指针: 目标 = *源 (类似Load)
        str = dstVal->getIRName() + " = *" + srcVal->getIRName();
    } else {
        // 其他情况 (两者都是指针，或两者都不是指针): 目标 = 源 (直接移动或指针地址赋值)
        str = dstVal->getIRName() + " = " + srcVal->getIRName();
    }
}
