///
/// @file BranchInstruction.h
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
#pragma once

#include "Instruction.h"

///
/// @brief 条件分支指令
///
class BranchInstruction : public Instruction {

public:
    /// @brief 构造函数
    /// @param _func 所属函数
    /// @param _condition 条件值
    /// @param _true_label 真出口标签
    /// @param _false_label 假出口标签
    BranchInstruction(Function * _func, Value * _condition, std::string _true_label, std::string _false_label);

    /// @brief 转换成字符串
    void toString(std::string & str) override;

    /// @brief 获取真出口标签
    /// @return 真出口标签
    std::string getTrueLabel() const { return true_label; }

    /// @brief 获取假出口标签
    /// @return 假出口标签
    std::string getFalseLabel() const { return false_label; }

private:
    /// @brief 真出口标签
    std::string true_label;

    /// @brief 假出口标签
    std::string false_label;
}; 