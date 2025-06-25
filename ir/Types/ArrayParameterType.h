///
/// @file ArrayParameterType.h
/// @brief 数组形参类型类，用于表示函数形参中的数组类型
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

#pragma once

#include <string>
#include <vector>

#include "Type.h"

///
/// @brief 数组形参类型类
/// 用于表示函数形参中的数组类型，区别于普通的ArrayType
/// 这种类型在IR中的格式为：元素类型 name[0] 或 元素类型 name[0][0]
/// 现在支持存储各个维度的具体大小信息
///
class ArrayParameterType final : public Type {
public:
    ///
    /// @brief 构造函数
    /// @param elementType 数组元素类型
    /// @param dimensionSizes 各个维度的大小向量
    ///
    ArrayParameterType(Type * elementType, const std::vector<int32_t>& dimensionSizes);

    ///
    /// @brief 获取数组元素类型
    /// @return Type* 元素类型
    ///
    Type * getElementType() const;

    ///
    /// @brief 获取数组维度数量
    /// @return int32_t 维度数量
    ///
    int32_t getDimensionCount() const;

    ///
    /// @brief 获取各个维度的大小
    /// @return const std::vector<int32_t>& 维度大小向量
    ///
    const std::vector<int32_t>& getDimensionSizes() const;

    ///
    /// @brief 判断是否是数组形参类型
    /// @return true
    ///
    bool isArrayParameterType() const override;

    ///
    /// @brief 获取IR字符串表示
    /// @return std::string IR字符串
    ///
    std::string toString() const override;

    ///
    /// @brief 获取带参数名的IR字符串表示
    /// @param paramName 参数名
    /// @return std::string IR字符串，格式为：元素类型 paramName[0] 或 元素类型 paramName[0][0]
    ///
    std::string toStringWithName(const std::string& paramName) const;

    ///
    /// @brief 获取类型大小
    /// @return int32_t 大小（指针大小）
    ///
    int32_t getSize() const override;

private:
    ///
    /// @brief 数组元素类型
    ///
    Type * elementType;

    ///
    /// @brief 各个维度的大小向量
    ///
    std::vector<int32_t> dimensionSizes;
};
