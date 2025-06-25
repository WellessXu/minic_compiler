///
/// @file GotoInstruction.cpp
/// @brief 无条件跳转指令即goto指令
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

#include "GotoInstruction.h"

///
/// @brief 无条件跳转指令的构造函数
/// @param target 跳转目标
///
GotoInstruction::GotoInstruction(Function * _func, Instruction * _target)
    : Instruction(_func, IRInstOperator::IRINST_OP_GOTO, VoidType::getType())
{
    // 真假目标一样，则无条件跳转
    target = static_cast<LabelInstruction *>(_target);
}

///
/// @brief 无条件跳转指令的构造函数，使用标签名称
/// @param _func 所属函数
/// @param _label_name 目标标签名称
///
GotoInstruction::GotoInstruction(Function * _func, std::string _label_name)
    : Instruction(_func, IRInstOperator::IRINST_OP_GOTO, VoidType::getType()),
      label_name(_label_name)
{
}

/// @brief 转换成IR指令文本
void GotoInstruction::toString(std::string & str)
{
    if (target) {
        str = "br label " + target->getIRName();
    } else if (!label_name.empty()) {
        str = "br label " + label_name;
    } else {
        str = "br <unknown>";
    }
}

///
/// @brief 获取目标Label指令
/// @return LabelInstruction* label指令
///
LabelInstruction * GotoInstruction::getTarget() const
{
    return target;
}
