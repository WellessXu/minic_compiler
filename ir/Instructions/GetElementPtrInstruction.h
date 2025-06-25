///
/// @file GetElementPtrInstruction.h
/// @brief 获取元素指针指令
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-23 <td>1.0     <td>zenglj  <td>新建
/// </table>
///

#pragma once

#include <vector>
#include "Instruction.h"
#include "PointerType.h"

///
/// @brief 获取元素指针指令类
/// 用于计算数组元素的地址，类似于LLVM的getelementptr指令
///
class GetElementPtrInstruction : public Instruction {

public:
    ///
    /// @brief 构造函数
    /// @param func 所属函数
    /// @param basePtr 基指针
    /// @param indices 索引列表
    /// @param resultType 结果类型（指针类型）
    ///
    GetElementPtrInstruction(Function *func, Value *basePtr, 
                           const std::vector<Value*> &indices, 
                           PointerType *resultType);

    ///
    /// @brief 转换成字符串显示
    /// @param str 转换后的字符串
    ///
    void toString(std::string & str) override;

private:
    ///
    /// @brief 索引数量
    ///
    size_t indexCount;
};

class Function;
class Value; 