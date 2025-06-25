///
/// @file LabelInstruction.h
/// @brief Label指令
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

class Function;

///
/// @brief Label指令
///
class LabelInstruction : public Instruction {

public:
    ///
    /// @brief 构造函数
    /// @param _func 所属函数
    ///
    explicit LabelInstruction(Function * _func);

    ///
    /// @brief 带标签名的构造函数
    /// @param _func 所属函数
    /// @param _label_name 标签名称
    ///
    LabelInstruction(Function * _func, std::string _label_name);

    ///
    /// @brief 转换成字符串
    /// @param str 返回指令字符串
    ///
    void toString(std::string & str) override;

private:
    ///
    /// @brief 标签名称
    ///
    std::string label_name;
};
