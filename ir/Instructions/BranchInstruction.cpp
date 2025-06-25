///
/// @file BranchInstruction.cpp
/// @brief 条件分支指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-25
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-25 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "BranchInstruction.h"
#include "VoidType.h"

/// @brief 构造函数
/// @param _func 所属函数
/// @param _condition 条件值
/// @param _true_label 真出口标签
/// @param _false_label 假出口标签
BranchInstruction::BranchInstruction(Function * _func, 
                                    Value * _condition, 
                                    std::string _true_label, 
                                    std::string _false_label)
    : Instruction(_func, IRInstOperator::IRINST_OP_BC, VoidType::getType()),
      true_label(_true_label),
      false_label(_false_label)
{
    addOperand(_condition);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BranchInstruction::toString(std::string & str)
{
    Value *condition = getOperand(0);

    // 条件分支指令
    str = "bc " + condition->getIRName() + ", label " + true_label + ", label " + false_label;
} 