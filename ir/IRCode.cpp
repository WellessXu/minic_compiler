///
/// @file IRCode.cpp
/// @brief IR指令序列类实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include "IRCode.h"
#include "Instruction.h" // Required for IRInstOperator and getOp()

/// @brief 析构函数
InterCode::~InterCode()
{
    Delete();
}

/// @brief 添加一个指令块，添加到尾部，并清除原来指令块的内容
/// @param block 指令块，请注意加入后会自动清空block的指令
void InterCode::addInst(InterCode & block)
{
    for (auto inst: block.code) {
        this->code.push_back(inst);
    }
    block.code.clear();
}

/// @brief 添加一条中间指令
/// @param inst IR指令
void InterCode::addInst(Instruction * inst)
{
    if (inst) {
        code.push_back(inst);
    }
}

/// @brief 获取指令序列
/// @return 指令序列
std::vector<Instruction *> & InterCode::getInsts()
{
    return code;
}

/// @brief 删除所有指令
void InterCode::Delete()
{
    // 不能直接删除指令，需要先清除操作数
    for (auto inst: code) {
        inst->clearOperands();
    }

    // 资源清理
    for (auto inst: code) {
        delete inst;
    }

    code.clear();
}

bool InterCode::hasTerminalInst() const
{
    if (code.empty()) {
        return false;
    }

    Instruction* lastInst = code.back();
    if (!lastInst) { // Defensive check, should not happen if insts are managed correctly
        return false;
    }

    IRInstOperator op = lastInst->getOp();
    return op == IRInstOperator::IRINST_OP_EXIT ||
           op == IRInstOperator::IRINST_OP_GOTO ||
           op == IRInstOperator::IRINST_OP_BC; // BC is for Branch Conditional
}
