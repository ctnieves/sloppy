#include "gbc_emu.hpp"

#include "dmg_rom.h"
#include "01_special.h"
#include "02_interrupts.h"
#include "03_op_sp_hl.h"
#include "04_op_r_imm.h"
#include "05_op_rp.h"
#include "06_ld_r_r.h"
#include "07_jr_jp_call_ret_rst.h"
#include "08_misc_instr.h"
#include "09_op_r_r.h"
#include "10_bit_ops.h"
#include "11_op_a_mhl.h"

gbc_emu::gbc_emu() {
    cpu.mmu = &mmu;
    ppu.mmu = &mmu;
    cpu.ppu = &ppu;

#if defined(SLOPPY_RENDER)
    SDL_Init(SDL_INIT_EVERYTHING);
    float scale = 2.0f;
    SDL_CreateWindowAndRenderer(SIZE_X*scale, SIZE_Y*scale, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, scale, scale);
    SDL_SetWindowTitle(window, "sloppy emu");
    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return;
    }
    ppu.renderer = renderer;
#endif

    gbc_mmu_init(&mmu);
    gbc_cpu_reset(&cpu);
    ppu_init(&ppu);
}

void gbc_emu::run() {
    gbc_cpu_set_boot_state(&cpu);
    gbc_cpu_loop(&cpu);
}

void gbc_emu::test() {
    memcpy(mmu.bios, DMG_ROM_bin, DMG_ROM_bin_len); mmu.in_bios = true;
    //gbc_cpu_set_boot_state(&cpu);
    cpu.mmu->io[0x00] = 0xFF;

    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ei_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/instr/daa.gb"); // PASSED

    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma/basic.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma/reg_read.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma/sources-GS.gb");

    // instr timing tests
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/add_sp_e_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_div-dmgABCmgb.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_div2-S.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_hwio-S.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_hwio-dmgABCmgb.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_regs-dmgABC.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_regs-mgb.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_regs-sgb.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/boot_regs-sgb2.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/call_cc_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/call_cc_timing2.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/call_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/call_timing2.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/di_timing-GS.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ei_sequence.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ei_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/halt_ime0_ei.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/halt_ime0_nointr_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/halt_ime1_timing.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/halt_ime1_timing2-GS.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/if_ie_registers.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/intr_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/jp_cc_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/jp_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ld_hl_sp_e_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma_restart.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma_start.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/oam_dma_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/pop_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/push_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/rapid_di_ei.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ret_cc_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ret_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/reti_intr_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/reti_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/rst_timing.gb");

    // timer tests
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/div_timing.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/div_write.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/rapid_toggle.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim00.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim01.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim10.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim11.gb"); // PASS
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim00_div_trigger.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim01_div_trigger.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim10_div_trigger.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tim11_div_trigger.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tima_reload.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tima_write_reloading.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/timer/tma_write_reloading.gb");

    // mbc tests
    //gbc_load_rom_file(&mmu, "./data/mbc1/bits_bank1.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/bits_bank2.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/pokemon_blue_gb.bin");
    //gbc_load_rom_file(&mmu, "./data/mbc1/bits_mode.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/bits_ramg.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/multicart_rom_8Mb.gb");
    //gbc_load_rom_file(&mmu, "./data/mbc1/ram_64kb.gb");
    //gbc_load_rom_file(&mmu, "./data/mbc1/ram_256kb.gb"); // PASSED

    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_512kb.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_1Mb.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_2Mb.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_4Mb.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_8Mb.gb"); // PASSED
    //gbc_load_rom_file(&mmu, "./data/mbc1/rom_16Mb.gb"); // PASSED

    // ppu tests
    //gbc_load_rom_file(&mmu, "data/m2_win_en_toggle.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_mode0_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_mode4_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_oam_ok_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_0_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_mode0_timing.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_mode0_timing_sprites.gb");
    //gbc_load_rom_file(&mmu, "data/mooneye-gb_hwtests/acceptance/ppu/intr_2_mode3_timing.gb");

    // interrupts
    //gbc_load_rom_file(&mmu, "./data/mooneye-gb_hwtests/acceptance/interrupts/ie_push.gb");
    //gbc_load_rom_file(&mmu, "./data/tests/interrupt_time/interrupt_time.gb");

    // mem-timing  - do this before interrupt timing?
    //gbc_load_rom_file(&mmu, "./data/tests/mem_timing/individual/01-read_timing.gb");
    //gbc_load_rom_file(&mmu, "./data/tests/mem_timing/individual/02-write_timing.gb");
    //gbc_load_rom_file(&mmu, "./data/tests/mem_timing/individual/03-modify_timing.gb");

    //gbc_load_rom_file(&mmu, "./DrMario.gb");
    //gbc_load_rom_file(&mmu, "./Tetris.gb");
    //gbc_load_rom_file(&mmu, "DK.gb");
    //gbc_load_rom_file(&mmu, "./MarioLand.gb"); // required mapper
    //gbc_load_rom_file(&mmu, "data/sprite_priority.gb");
    //gbc_load_rom_file(&mmu, "data/tests/oam_bug/rom_singles/4-scanline_timing.gb");

    // CPU instruction tests
    //gbc_load_rom_file(&mmu, "data/tests/oam_bug/oam_bug.gb");
    gbc_load_rom_file(&mmu, "data/tests/cpu_instrs/cpu_instrs.gb");
    //gbc_load_rom_file(&mmu, "data/tests/instr_timing/instr_timing.gb");
    gbc_cpu_loop(&cpu);

#if defined(SLOPPY_RENDER)
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}