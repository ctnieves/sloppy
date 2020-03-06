#pragma once

#include "types.h"
#include "gbc_gpu.h"
#include "gbc_mmu.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

typedef struct {
    // two types of timeclocks in the Z80
    int m;
    int t;
} gbc_clock;

/*
 * 16bit Hi   Lo   Name/Function
 * AF    A    -    Accumulator & Flags
 * BC    B    C    BC
 * DE    D    E    DE
 * HL    H    L    HL
 * SP    -    -    Stack Pointer
 * PC    -    -    Program Counter/Pointer
 * As shown above, most registers can be accessed either as one 16bit
 * register, or as two separate 8bit registers.
 */
typedef struct {
    u16 pc, sp;
    u8 a, b, c, d, e, h, l, f; // 8-bit registers
    //u8 t, m; // Clock for last instruction
    gbc_clock clk;
} gbc_cpu_registers;

// save register set
typedef struct {
    u8 a, b, c, d, e, h, l, f;
} gbc_cpu_rsv;

typedef struct {
    gbc_clock clk;
    gbc_cpu_registers registers;
    gbc_cpu_rsv rsv;

    u8 HALT;

    // interrupts
    u8 IME; // IME - Interrupt Master Enable Flag (Write Only)
    u8 IE; // IE - Interrupt Enable (R/W)
    u8 IF; // IF - Interrupt Flag (R/W)

    gbc_mmu *mmu;
    gbc_gpu *gpu;
} gbc_cpu;

void gbc_cpu_reset(gbc_cpu *cpu);
void gbc_cpu_loop(gbc_cpu *cpu);
