///
/// @file NegInstruction.cpp
/// @brief 整数求负指令
///
/// @author Claude
/// @version 1.0
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-23 <td>1.0     <td>Claude  <td>新建
/// </table>
///

#include "NegInstruction.h"

/// @brief 构造函数
/// @param _func 函数
/// @param _srcVal 源操作数
/// @param _type 类型
NegInstruction::NegInstruction(Function * _func, Value * _srcVal, Type * _type)
    : Instruction(_func, IRInstOperator::IRINST_OP_NEG_I, _type)
{
    // 添加源操作数作为使用
    addOperand(_srcVal);
}

/// @brief 转换成字符串
/// @param str 保存字符串
void NegInstruction::toString(std::string & str)
{
    // 清空字符串
    str.clear();

    Value *src = getOperand(0);

    // 有返回值或结果
    if (hasResultValue()) {
        str = getIRName() + " = neg " + src->getIRName();
    } else {
        str = "neg " + src->getIRName();
    }
}