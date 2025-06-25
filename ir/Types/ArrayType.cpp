///
/// @file ArrayType.cpp
/// @brief 数组类型的实现
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

#include "ArrayType.h"
#include <sstream>

///
/// @brief 构造函数
/// @param elementType 数组元素类型
/// @param dimensions 数组各维度大小
///
ArrayType::ArrayType(Type * elementType, const std::vector<int32_t> & dimensions)
    : Type(ArrayTyID), elementType(elementType), dimensions(dimensions)
{
}

///
/// @brief 获取数组总大小（所有元素的数量）
/// @return int32_t 总元素数量
///
int32_t ArrayType::getTotalElementCount() const
{
    int32_t total = 1;
    for (int32_t dim : dimensions) {
        if (dim <= 0) {
            return 0; // 形参数组第一维可能为0
        }
        total *= dim;
    }
    return total;
}

///
/// @brief 获取数组所占内存空间大小
/// @return int32_t 内存大小（字节）
///
int32_t ArrayType::getSize() const
{
    int32_t elementSize = elementType->getSize();
    if (elementSize <= 0) {
        return -1;
    }
    
    int32_t totalElements = getTotalElementCount();
    if (totalElements <= 0) {
        return 0; // 形参数组或无效数组
    }
    
    return totalElements * elementSize;
}

///
/// @brief 转换为字符串表示
/// @return std::string 字符串表示
///
std::string ArrayType::toString() const
{
    std::ostringstream oss;
    oss << elementType->toString();
    
    // 不在这里添加维度信息，维度信息应该在变量声明时添加
    // 这样可以生成 "i32 %var[5]" 而不是 "i32[5] %var"
    
    return oss.str();
}

///
/// @brief 创建数组类型的静态方法
/// @param elementType 元素类型
/// @param dimensions 维度大小数组
/// @return ArrayType* 数组类型指针
///
ArrayType * ArrayType::get(Type * elementType, const std::vector<int32_t> & dimensions)
{
    return new ArrayType(elementType, dimensions);
} 