///
/// @file GotoInstruction.h
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
#pragma once

#include <string>

#include "Instruction.h"
#include "LabelInstruction.h"
#include "Function.h"

///
/// @brief 无条件跳转指令
///
class GotoInstruction final : public Instruction {

public:
    ///
    /// @brief 无条件跳转指令的构造函数
    /// @param target 跳转目标
    ///
    GotoInstruction(Function * _func, Instruction * _target);

    ///
    /// @brief 无条件跳转指令的构造函数，使用标签名称
    /// @param _func 所属函数
    /// @param _label_name 目标标签名称
    ///
    GotoInstruction(Function * _func, std::string _label_name);

    /// @brief 转换成字符串
    void toString(std::string & str) override;

    ///
    /// @brief 获取目标Label指令
    /// @return LabelInstruction*
    ///
    [[nodiscard]] LabelInstruction * getTarget() const;

private:
    ///
    /// @brief 跳转到的目标Label指令
    ///
    LabelInstruction * target = nullptr;

    ///
    /// @brief 目标标签名称
    ///
    std::string label_name;
};
