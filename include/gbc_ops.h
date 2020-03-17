#pragma once

#include "gbc_cpu.h"

typedef void (*gbc_operator)(gbc_cpu *cpu);

void execute_op(gbc_cpu *cpu, u8 opcode);
void execute_cb_op(gbc_cpu *cpu);
const char* op_string(u8 opcode);

void LD_r8_r8(gbc_cpu *cpu, u8 *r1, u8 *r2);
void LD_r8_u8(gbc_cpu *cpu, u8 *r1, u8 v);
void LD_r8_mHL(gbc_cpu *cpu, u8 *r);
void LD_mHL_u8(gbc_cpu *cpu, u8 v);
void LD_mBC_A(gbc_cpu *cpu);
void LD_mDE_A(gbc_cpu *cpu);
void LD_mHLI_A(gbc_cpu *cpu);
void LD_mC_A(gbc_cpu *cpu);
void LD_mHLD_A(gbc_cpu *cpu);
void LD_HL_d8(gbc_cpu *cpu);
void LD_A_mBC(gbc_cpu *cpu);
void LD_A_mDE(gbc_cpu *cpu);
void LD_A_a16(gbc_cpu *cpu);
void LD_a16_A(gbc_cpu *cpu);
void LD_a16_SP(gbc_cpu *cpu);
void LD_BC_d16(gbc_cpu *cpu);
void LD_DE_d16(gbc_cpu *cpu);
void LD_HL_d16(gbc_cpu *cpu);
void LD_SP_HL(gbc_cpu *cpu);
void LD_SP_d16(gbc_cpu *cpu);
void LD_A_mHLI(gbc_cpu *cpu);
void LD_A_mHLD(gbc_cpu *cpu);
void LDH_a8_A(gbc_cpu *cpu);
void LDH_A_a8(gbc_cpu *cpu);
void LD_HL_SP_r8(gbc_cpu *cpu);
void LD_A_mC(gbc_cpu *cpu);
void ADD_u8(gbc_cpu *cpu, u8 v);
void ADD_HL_BC(gbc_cpu *cpu);
void ADD_HL_DE(gbc_cpu *cpu);
void ADD_HL_HL(gbc_cpu *cpu);
void ADD_HL_SP(gbc_cpu *cpu);
void ADD_SP_d8(gbc_cpu *cpu);
void ADC_A_u8(gbc_cpu *cpu, u8 v);
void SUB_u8(gbc_cpu *cpu, u8 v);
void SBC_A_u8(gbc_cpu *cpu, u8 v);
void AND_u8(gbc_cpu *cpu, u8 v);
void DAA(gbc_cpu *cpu);
void XOR_u8(gbc_cpu *cpu, u8 v);
void OR_u8(gbc_cpu *cpu, u8 v);
void CP_u8(gbc_cpu *cpu, u8 v);
void INC_r8(gbc_cpu *cpu, u8 *r);
void INC_mHL(gbc_cpu *cpu);
void INC_BC(gbc_cpu *cpu);
void INC_DE(gbc_cpu *cpu);
void INC_HL(gbc_cpu *cpu);
void INC_SP(gbc_cpu *cpu);
void DEC_r8(gbc_cpu *cpu, u8 *r);
void DEC_mHL(gbc_cpu *cpu);
void DEC_BC(gbc_cpu *cpu);
void DEC_DE(gbc_cpu *cpu);
void DEC_HL(gbc_cpu *cpu);
void DEC_SP(gbc_cpu *cpu);
void RLA(gbc_cpu *cpu);
void RLCA(gbc_cpu *cpu);
void RRA(gbc_cpu *cpu);
void RRCA(gbc_cpu *cpu);
void CPL(gbc_cpu *cpu);
void NEG(gbc_cpu *cpu);
void CCF(gbc_cpu *cpu);
void PUSH_BC(gbc_cpu *cpu);
void PUSH_DE(gbc_cpu *cpu);
void PUSH_HL(gbc_cpu *cpu);
void PUSH_AF(gbc_cpu *cpu);
void POP_BC(gbc_cpu *cpu);
void POP_DE(gbc_cpu *cpu);
void POP_HL(gbc_cpu *cpu);
void POP_AF(gbc_cpu *cpu);
void JP_a16(gbc_cpu *cpu);
void JP_mHL(gbc_cpu *cpu);
void JP_NZ_a16(gbc_cpu *cpu);
void JP_Cnn(gbc_cpu *cpu);
void JR_r8(gbc_cpu *cpu);
void JR_NZ_r8(gbc_cpu *cpu);
void JR_Z_r8(gbc_cpu *cpu) ;
void JR_NC_r8(gbc_cpu *cpu);
void JR_C_r8(gbc_cpu *cpu) ;
void CALL_a16(gbc_cpu *cpu);
void CALL_NZ_a16(gbc_cpu *cpu);
void CALL_Z_a16(gbc_cpu *cpu);
void CALL_NC_a16(gbc_cpu *cpu);
void CALL_C_a16(gbc_cpu *cpu);
void RET(gbc_cpu *cpu);
void RET_I(gbc_cpu *cpu);
void RET_NZ(gbc_cpu *cpu);
void RET_Z(gbc_cpu *cpu);
void RET_NC(gbc_cpu *cpu);
void RET_C(gbc_cpu *cpu);
void JP_C_a16(gbc_cpu *cpu);
void JP_Z_a16(gbc_cpu *cpu);
void JP_NC_a16(gbc_cpu *cpu);
void RST_u8(gbc_cpu *cpu, u8 v);
void RST_00H(gbc_cpu *cpu);
void RST_08H(gbc_cpu *cpu);
void RST_10H(gbc_cpu *cpu);
void RST_18H(gbc_cpu *cpu);
void RST_20H(gbc_cpu *cpu);
void RST_28H(gbc_cpu *cpu);
void RST_30H(gbc_cpu *cpu);
void RST_38H(gbc_cpu *cpu);
void NOP(gbc_cpu *cpu);
void HALT(gbc_cpu *cpu);
void DI(gbc_cpu *cpu);
void EI(gbc_cpu *cpu);
void rsv(gbc_cpu *cpu);
void rrs(gbc_cpu *cpu);
void PREFIX_CB(gbc_cpu *cpu);
void XX(gbc_cpu *cpu);
