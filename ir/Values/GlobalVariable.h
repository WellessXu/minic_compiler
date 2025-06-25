///
/// @file GlobalVariable.h
/// @brief 全局变量描述类
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

#include "GlobalValue.h"
#include "IRConstant.h"
#include "ArrayType.h"

///
/// @brief 全局变量，寻址时通过符号名或变量名来寻址
///
class GlobalVariable : public GlobalValue {

public:
    ///
    /// @brief 构建全局变量，默认对齐为4字节
    /// @param _type 类型
    /// @param _name 名字
    /// @param _initializer 初始化常量（可选）
    ///
    explicit GlobalVariable(Type * _type, std::string _name, Constant* _initializer = nullptr) : GlobalValue(_type, _name), initializer(_initializer)
    {
        // 设置对齐大小
        setAlignment(4);
        
        // 如果有初始化器，则不在BSS段
        if (_initializer != nullptr) {
            inBSSSection = false;
        }
    }

    ///
    /// @brief  检查是否是函数
    /// @return true 是函数
    /// @return false 不是函数
    ///
    [[nodiscard]] bool isGlobalVarible() const override
    {
        return true;
    }

    ///
    /// @brief 是否属于BSS段的变量，即未初始化过的变量，或者初值都为0的变量
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isInBSSSection() const
    {
        return this->inBSSSection;
    }

    ///
    /// @brief 获取初始化器
    /// @return 初始化器常量，如果没有则返回nullptr
    ///
    [[nodiscard]] Constant* getInitializer() const
    {
        return initializer;
    }

    ///
    /// @brief 设置初始化器
    /// @param _initializer 初始化器常量
    ///
    void setInitializer(Constant* _initializer)
    {
        initializer = _initializer;
        if (_initializer != nullptr) {
            inBSSSection = false;
        } else {
            inBSSSection = true;
        }
    }

    ///
    /// @brief 检查是否有初始化器
    /// @return true 如果有初始化器
    ///
    [[nodiscard]] bool hasInitializer() const
    {
        return initializer != nullptr;
    }

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    int32_t getScopeLevel() override
    {
        return 0;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    int32_t getLoadRegId() override
    {
        return this->loadRegNo;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    void setLoadRegId(int32_t regId) override
    {
        this->loadRegNo = regId;
    }

    ///
    /// @brief Declare指令IR显示
    /// @param str
    ///
    void toDeclareString(std::string & str)
    {
        if (getType()->isArrayType()) {
            // 数组类型需要特殊处理
            ArrayType* arrayType = dynamic_cast<ArrayType*>(getType());
            std::string elementTypeStr = arrayType->getElementType()->toString();
            
            // 构建维度字符串
            std::string dimensionStr;
            const auto& dimensions = arrayType->getDimensions();
            for (size_t i = 0; i < dimensions.size(); i++) {
                dimensionStr += "[" + std::to_string(dimensions[i]) + "]";
            }
            
            str = "declare " + elementTypeStr + " " + getIRName() + dimensionStr;
        } else {
            str = "declare " + getType()->toString() + " " + getIRName();
            
            // 添加初始化器支持
            if (hasInitializer()) {
                str += " = " + initializer->getIRName();
            } else if (!hasInitializer()) {
                // 对于未初始化的全局变量，不显示初始化器
                // 但为了与用户期望的格式一致，未初始化的可以不显示"= 0"
            }
        }
    }

    ///
    /// @brief 生成全局变量定义的IR字符串
    /// @param str 输出字符串
    ///
    void toDefineString(std::string & str)
    {
        str = getIRName() + " = global " + getType()->toString();
        if (hasInitializer()) {
            str += " " + initializer->getIRName();
        } else {
            // 默认初始化为0
            if (getType()->isIntegerType()) {
                str += " 0";
            } else {
                str += " undef";
            }
        }
    }

private:
    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;

    ///
    /// @brief 默认全局变量在BSS段，没有初始化，或者即使初始化过，但都值都为0
    ///
    bool inBSSSection = true;

    ///
    /// @brief 全局变量的初始化器
    ///
    Constant* initializer = nullptr;
};
