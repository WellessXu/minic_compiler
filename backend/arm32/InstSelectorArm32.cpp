﻿///
/// @file InstSelectorArm32.cpp
/// @brief 指令选择器-ARM32的实现
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
#include <cstdio>

#include "Common.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "PlatformArm32.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "BranchInstruction.h"
#include "NegInstruction.h"

/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm32::InstSelectorArm32(vector<Instruction *> & _irCode,
                                     ILocArm32 & _iloc,
                                     Function * _func,
                                     SimpleRegisterAllocator & allocator)
    : ir(_irCode), iloc(_iloc), func(_func), simpleRegisterAllocator(allocator)
{
    translator_handlers[IRInstOperator::IRINST_OP_ENTRY] = &InstSelectorArm32::translate_entry;
    translator_handlers[IRInstOperator::IRINST_OP_EXIT] = &InstSelectorArm32::translate_exit;

    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm32::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm32::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm32::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm32::translate_add_int32;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm32::translate_sub_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm32::translate_mul_int32;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm32::translate_div_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm32::translate_mod_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NEG_I] = &InstSelectorArm32::translate_neg_int32;

    translator_handlers[IRInstOperator::IRINST_OP_GT_I] = &InstSelectorArm32::translate_gt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_GE_I] = &InstSelectorArm32::translate_ge_int32;
    translator_handlers[IRInstOperator::IRINST_OP_LT_I] = &InstSelectorArm32::translate_lt_int32;
    translator_handlers[IRInstOperator::IRINST_OP_LE_I] = &InstSelectorArm32::translate_le_int32;
    translator_handlers[IRInstOperator::IRINST_OP_EQ_I] = &InstSelectorArm32::translate_eq_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NE_I] = &InstSelectorArm32::translate_ne_int32;

    translator_handlers[IRInstOperator::IRINST_OP_BC] = &InstSelectorArm32::translate_bc;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
void InstSelectorArm32::run()
{
    for (auto inst: ir) {

        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
        }
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // 操作符
    IRInstOperator op = inst->getOp();

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm32::outputIRInstruction(Instruction * inst)
{
    std::string irStr;
    inst->toString(irStr);
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    iloc.label(labelInst->getName());
}

/// @brief goto指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_goto(Instruction * inst)
{
    Instanceof(gotoInst, GotoInstruction *, inst);

    // 无条件跳转
    iloc.jump(gotoInst->getTarget()->getName());
}

/// @brief 函数入口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_entry(Instruction * inst)
{
    // 查看保护的寄存器
    auto & protectedRegNo = func->getProtectedReg();
    auto & protectedRegStr = func->getProtectedRegStr();

    bool first = true;
    for (auto regno: protectedRegNo) {
        if (first) {
            protectedRegStr = PlatformArm32::regName[regno];
            first = false;
        } else {
            protectedRegStr += "," + PlatformArm32::regName[regno];
        }
    }

    if (!protectedRegStr.empty()) {
        iloc.inst("push", "{" + protectedRegStr + "}");
    }

    // 为fun分配栈帧，含局部变量、函数调用值传递的空间等
    iloc.allocStack(func, ARM32_TMP_REG_NO);
}

/// @brief 函数出口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_exit(Instruction * inst)
{
    if (inst->getOperandsNum()) {
        // 存在返回值
        Value * retVal = inst->getOperand(0);

        // 赋值给寄存器R0
        iloc.load_var(0, retVal);
    }

    // 恢复栈空间
    iloc.inst("mov", "sp", "fp");

    // 保护寄存器的恢复
    auto & protectedRegStr = func->getProtectedRegStr();
    if (!protectedRegStr.empty()) {
        iloc.inst("pop", "{" + protectedRegStr + "}");
    }

    iloc.inst("bx", "lr");
}

/// @brief 赋值指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        // 寄存器 => 内存
        // 寄存器 => 寄存器

        // r8 -> rs 可能用到r9
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 内存变量 => 寄存器

        iloc.load_var(result_regId, arg1);
    } else {
        // 内存变量 => 内存变量

        int32_t temp_regno = simpleRegisterAllocator.Allocate();

        // arg1 -> r8
        iloc.load_var(temp_regno, arg1);

        // r8 -> rs 可能用到r9
        iloc.store_var(temp_regno, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief 二元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm32::translate_two_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 + r9 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 整数加法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
}

/// @brief 整数乘法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mul_int32(Instruction * inst)
{
    translate_two_operator(inst, "mul");
}

/// @brief 整数除法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_div_int32(Instruction * inst)
{
    // ARM汇编中没有直接的整数除法指令，通常使用库函数或特殊指令序列
    // 这里简化处理，使用sdiv指令（在ARMv7-A架构及以上支持）
    translate_two_operator(inst, "sdiv");
}

/// @brief 整数求余指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mod_int32(Instruction * inst)
{
    // ARM没有直接的求余指令，通常需要先做除法再用乘法和减法计算余数
    // 这里我们需要实现：result = arg1 - (arg1 / arg2) * arg2
    
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);
    
    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;
    int32_t temp_reg_no;
    
    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {
        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);
        // arg1 -> r8
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }
    
    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {
        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);
        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }
    
    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
    if (result_reg_no == -1) {
        // 分配一个寄存器，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }
    
    // 分配一个临时寄存器用于存储中间结果
    temp_reg_no = simpleRegisterAllocator.Allocate();
    
    // r8 / r9 -> temp_reg （计算arg1 / arg2）
    iloc.inst("sdiv", 
              PlatformArm32::regName[temp_reg_no], 
              PlatformArm32::regName[load_arg1_reg_no], 
              PlatformArm32::regName[load_arg2_reg_no]);
    
    // temp_reg * r9 -> temp_reg （计算(arg1 / arg2) * arg2）
    iloc.inst("mul", 
              PlatformArm32::regName[temp_reg_no], 
              PlatformArm32::regName[temp_reg_no], 
              PlatformArm32::regName[load_arg2_reg_no]);
    
    // r8 - temp_reg -> r10 （计算arg1 - (arg1 / arg2) * arg2）
    iloc.inst("sub", 
              PlatformArm32::regName[load_result_reg_no], 
              PlatformArm32::regName[load_arg1_reg_no], 
              PlatformArm32::regName[temp_reg_no]);
    
    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {
        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }
    
    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
    simpleRegisterAllocator.free(temp_reg_no);
}

/// @brief 通用比较函数，生成比较指令
/// @param inst IR指令
/// @param condition 条件代码 (eq, ne, gt, ge, lt, le)
void InstSelectorArm32::translate_compare(Instruction * inst, const string & condition)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);
    
    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;
    
    // 处理第一个操作数
    if (arg1_reg_no == -1) {
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }
    
    // 处理第二个操作数
    if (arg2_reg_no == -1) {
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }
    
    // 处理结果
    if (result_reg_no == -1) {
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }
    
    // 先置0
    iloc.inst("mov", 
              PlatformArm32::regName[load_result_reg_no], 
              "#0");
    
    // 比较两个操作数
    iloc.inst("cmp", 
              PlatformArm32::regName[load_arg1_reg_no], 
              PlatformArm32::regName[load_arg2_reg_no]);
    
    // 根据条件设置结果为1
    iloc.inst("mov" + condition, 
              PlatformArm32::regName[load_result_reg_no], 
              "#1");
    
    // 存储结果
    if (result_reg_no == -1) {
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }
    
    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 大于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_gt_int32(Instruction * inst)
{
    // 大于比较: ARM32中使用CMP+GT条件
    translate_compare(inst, "gt");
}

/// @brief 大于等于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_ge_int32(Instruction * inst)
{
    // 大于等于比较: ARM32中使用CMP+GE条件
    translate_compare(inst, "ge");
}

/// @brief 小于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_lt_int32(Instruction * inst)
{
    // 小于比较: ARM32中使用CMP+LT条件
    translate_compare(inst, "lt");
}

/// @brief 小于等于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_le_int32(Instruction * inst)
{
    // 小于等于比较: ARM32中使用CMP+LE条件
    translate_compare(inst, "le");
}

/// @brief 等于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_eq_int32(Instruction * inst)
{
    // 等于比较: ARM32中使用CMP+EQ条件
    translate_compare(inst, "eq");
}

/// @brief 不等于比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_ne_int32(Instruction * inst)
{
    // 不等于比较: ARM32中使用CMP+NE条件
    translate_compare(inst, "ne");
}

/// @brief 条件分支指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_bc(Instruction * inst)
{
    // 从bc指令中获取条件变量和标签
    Instanceof(branchInst, BranchInstruction *, inst);
    
    if (!branchInst) {
        minic_log(LOG_ERROR, "转换BranchInstruction失败");
        return;
    }
    
    // 获取条件变量和标签名称
    Value * condVar = inst->getOperand(0);
    std::string trueLabelName = branchInst->getTrueLabel();
    std::string falseLabelName = branchInst->getFalseLabel();
    if (!condVar) {
        minic_log(LOG_ERROR, "获取失败");
        return;
    }
    // 检查条件变量是否在寄存器中，如果不在则加载到寄存器
    int32_t condReg = condVar->getRegId();
    int32_t loadCondReg;
    
    if (condReg == -1) {
        loadCondReg = simpleRegisterAllocator.Allocate(condVar);
        iloc.load_var(loadCondReg, condVar);
		
    } else {
        loadCondReg = condReg;
		minic_log(LOG_ERROR, "1");
    }
    
    // 比较条件变量与0
    iloc.inst("cmp", PlatformArm32::regName[loadCondReg], "#0");
    
    // 条件为真时跳转到真标签
    iloc.inst("bne", trueLabelName);
    
    // 条件为假时跳转到假标签
    iloc.jump(falseLabelName);
    
    // 如果分配了寄存器，则释放
    if (condReg == -1) {
        simpleRegisterAllocator.free(condVar);
    }
	
}

/// @brief 函数调用指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);

    int32_t operandNum = callInst->getOperandsNum();

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    if (operandNum) {

        // 强制占用这几个寄存器参数传递的寄存器
        simpleRegisterAllocator.Allocate(0);
        simpleRegisterAllocator.Allocate(1);
        simpleRegisterAllocator.Allocate(2);
        simpleRegisterAllocator.Allocate(3);

        // 前四个的后面参数采用栈传递
        int esp = 0;
        for (int32_t k = 4; k < operandNum; k++) {

            auto arg = callInst->getOperand(k);

            // 新建一个内存变量，用于栈传值到形参变量中
            MemVariable * newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
            newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
            esp += 4;

            Instruction * assignInst = new MoveInstruction(func, newVal, arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }

        for (int32_t k = 0; k < operandNum && k < 4; k++) {

            auto arg = callInst->getOperand(k);

            // 检查实参的类型是否是临时变量。
            // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
            // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

            Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }
    }

    iloc.call_fun(callInst->getName());

    if (operandNum) {
        simpleRegisterAllocator.free(0);
        simpleRegisterAllocator.free(1);
        simpleRegisterAllocator.free(2);
        simpleRegisterAllocator.free(3);
    }

    // 赋值指令
    if (callInst->hasResultValue()) {

        // 新建一个赋值操作
        Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm32::intRegVal[0]);

        // 翻译赋值指令
        translate_assign(assignInst);

        delete assignInst;
    }

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

///
/// @brief 实参指令翻译成ARM32汇编
/// @param inst
///
void InstSelectorArm32::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 4) {
        // 前四个参数
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 必须是内存分配，若不是则出错
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM32_SP_REG_NO)) {

            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        }
    }

    realArgCount++;
}

/// @brief 整数求负指令翻译成ARM32汇编 (实现为0-操作数)
/// @param inst IR指令
void InstSelectorArm32::translate_neg_int32(Instruction * inst)
{
    Instanceof(negInst, NegInstruction *, inst);

    // 获取操作数
    Value * operand = negInst->getOperand(0);

    // 为结果分配寄存器
    int result_reg = simpleRegisterAllocator.Allocate(negInst);

    // 加载操作数到临时寄存器
    int operand_reg = simpleRegisterAllocator.Allocate(operand);
    iloc.load_var(operand_reg, operand);

    // 生成指令: 使用rsb (reverse subtract) 实现0-操作数
    iloc.inst("rsb", PlatformArm32::regName[result_reg], PlatformArm32::regName[operand_reg], "#0");

    // 如果需要保存结果
    if (negInst->hasResultValue()) {
        iloc.store_var(result_reg, negInst, ARM32_TMP_REG_NO);
    }

    // 释放临时寄存器
    simpleRegisterAllocator.free(operand_reg);
}
