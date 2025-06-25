///
/// @file ArrayParameterType.cpp
/// @brief 数组形参类型类实现
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新建
/// </table>
///

#include "ArrayParameterType.h"

/// @brief 构造函数
/// @param elementType 数组元素类型
/// @param dimensionSizes 各个维度的大小向量
ArrayParameterType::ArrayParameterType(Type * elementType, const std::vector<int32_t>& dimensionSizes)
    : Type(TypeID::ArrayParameterTyID), elementType(elementType), dimensionSizes(dimensionSizes)
{
}

/// @brief 获取数组元素类型
/// @return Type* 元素类型
Type * ArrayParameterType::getElementType() const
{
    return elementType;
}

/// @brief 获取数组维度数量
/// @return int32_t 维度数量
int32_t ArrayParameterType::getDimensionCount() const
{
    return dimensionSizes.size();
}

/// @brief 获取各个维度的大小
/// @return const std::vector<int32_t>& 维度大小向量
const std::vector<int32_t>& ArrayParameterType::getDimensionSizes() const
{
    return dimensionSizes;
}

/// @brief 判断是否是数组形参类型
/// @return true
bool ArrayParameterType::isArrayParameterType() const
{
    return true;
}

/// @brief 获取IR字符串表示
/// @return std::string IR字符串
std::string ArrayParameterType::toString() const
{
    return elementType->toString();
}

/// @brief 获取带参数名的IR字符串表示
/// @param paramName 参数名
/// @return std::string IR字符串，格式为：元素类型 paramName[0] 或 元素类型 paramName[0][2]
std::string ArrayParameterType::toStringWithName(const std::string& paramName) const
{
    std::string result = elementType->toString() + " " + paramName;
    for (size_t i = 0; i < dimensionSizes.size(); i++) {
        result += "[" + std::to_string(dimensionSizes[i]) + "]";
    }
    return result;
}

/// @brief 获取类型大小
/// @return int32_t 大小（指针大小）
int32_t ArrayParameterType::getSize() const
{
    // 数组形参实际上是指针，所以返回指针大小
    return 8; // 64位系统指针大小
} 