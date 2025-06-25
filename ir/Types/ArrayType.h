///
/// @file ArrayType.h
/// @brief 数组类型的定义
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
#include "Type.h"

///
/// @brief 数组类型类
///
class ArrayType : public Type {

public:
    ///
    /// @brief 构造函数
    /// @param elementType 数组元素类型
    /// @param dimensions 数组各维度大小
    ///
    ArrayType(Type * elementType, const std::vector<int32_t> & dimensions);

    ///
    /// @brief 析构函数
    ///
    ~ArrayType() override = default;

    ///
    /// @brief 获取数组元素类型
    /// @return Type* 元素类型指针
    ///
    [[nodiscard]] Type * getElementType() const
    {
        return elementType;
    }

    ///
    /// @brief 获取数组维度数量
    /// @return int32_t 维度数量
    ///
    [[nodiscard]] int32_t getDimensionCount() const
    {
        return static_cast<int32_t>(dimensions.size());
    }

    ///
    /// @brief 获取指定维度的大小
    /// @param index 维度索引
    /// @return int32_t 该维度的大小
    ///
    [[nodiscard]] int32_t getDimensionSize(int32_t index) const
    {
        if (index >= 0 && index < static_cast<int32_t>(dimensions.size())) {
            return dimensions[index];
        }
        return -1;
    }

    ///
    /// @brief 获取所有维度大小
    /// @return const std::vector<int32_t>& 维度大小数组
    ///
    [[nodiscard]] const std::vector<int32_t> & getDimensions() const
    {
        return dimensions;
    }

    ///
    /// @brief 获取数组总大小（所有元素的数量）
    /// @return int32_t 总元素数量
    ///
    [[nodiscard]] int32_t getTotalElementCount() const;

    ///
    /// @brief 获取数组所占内存空间大小
    /// @return int32_t 内存大小（字节）
    ///
    [[nodiscard]] int32_t getSize() const override;

    ///
    /// @brief 转换为字符串表示
    /// @return std::string 字符串表示
    ///
    [[nodiscard]] std::string toString() const override;

    ///
    /// @brief 创建数组类型的静态方法
    /// @param elementType 元素类型
    /// @param dimensions 维度大小数组
    /// @return ArrayType* 数组类型指针
    ///
    static ArrayType * get(Type * elementType, const std::vector<int32_t> & dimensions);

private:
    /// @brief 数组元素类型
    Type * elementType;

    /// @brief 数组各维度大小
    std::vector<int32_t> dimensions;
}; 