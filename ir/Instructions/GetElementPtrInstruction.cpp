///
/// @file GetElementPtrInstruction.cpp
/// @brief 获取元素指针指令实现
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

#include "GetElementPtrInstruction.h"
#include "Function.h"
#include "PointerType.h"
#include "Common.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param _basePtr 基址指针
/// @param _indices 索引列表
/// @param _resultType 结果类型（指针类型）
///
GetElementPtrInstruction::GetElementPtrInstruction(Function *func, Value *basePtr, 
                                                    const std::vector<Value*> &indices, 
                                                    PointerType *resultType)
    : Instruction(func, IRInstOperator::IRINST_OP_GEP, resultType), indexCount(indices.size())
{
    // 添加基址指针作为第一个操作数
    addOperand(basePtr);
    
    // 添加所有索引作为操作数
    for (auto index : indices) {
        addOperand(index);
    }
}

///
/// @brief 转换成字符串显示
/// @param str 转换后的字符串
///
void GetElementPtrInstruction::toString(std::string & str)
{
    Value * basePtr = getOperand(0);
    
    str = getIRName() + " = getelementptr " + basePtr->getIRName();
    
    // 添加索引
    for (size_t i = 1; i <= indexCount; i++) {
        Value * index = getOperand(i);
        str += "[" + index->getIRName() + "]";
    }
} 