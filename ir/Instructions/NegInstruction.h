///
/// @file NegInstruction.h
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
#pragma once

#include "Instruction.h"

///
/// @brief 求负指令(NEG)
///
class NegInstruction : public Instruction {

public:
    /// @brief 构造函数
    /// @param _func 函数
    /// @param _srcVal 源操作数
    /// @param _type 类型
    NegInstruction(Function * _func, Value * _srcVal, Type * _type);

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};