#pragma once

#define MEM_OAM 0xFE00
// 0000-3FFF   16KB ROM Bank 00     (in cartridge, fixed at bank 00)
// 4000-7FFF   16KB ROM Bank 01..NN (in cartridge, switchable bank number)
// 8000-9FFF   8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
// A000-BFFF   8KB External RAM     (in cartridge, switchable bank, if any)
// C000-CFFF   4KB Work RAM Bank 0 (WRAM)
// D000-DFFF   4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
// E000-FDFF   Same as C000-DDFF (ECHO)    (typically not used)
// FE00-FE9F   Sprite Attribute Table (OAM)
// FEA0-FEFF   Not Usable
// FF00-FF7F   I/O Ports
// FF80-FFFE   High RAM (HRAM)
// FFFF        Interrupt Enable Register
typedef struct {
    u8 bios[0x100];
    u8 rom[0x8000];
    u8 vram[0x2000];
    u8 wram[0x4000];
    u8 echo[0x4000];
    u8 oam[0xA0];
    u8 io[0x80];
    u8 hram[0x80];
    u8 zram[0xFFFF];

    bool in_bios;
} gbc_mmu;

const u8 kInitialValuesForFFXX[256];
const u8 kInitialValuesForColorFFXX[256];

void gbc_mmu_init(gbc_mmu *mmu);
u8* get_address_ptr(gbc_mmu *mmu , u16 address);

u8 read_u8(gbc_mmu *mmu , u16 address);
void write_u8(gbc_mmu *mmu , u16 address, u8 val);
u16 read_u16(gbc_mmu *mmu , u16 address);
void write_u16(gbc_mmu *mmu , u16 address, u16 val);

bool read_bit(gbc_mmu *mmu , u16 address, u8 bit);
void set_bit(gbc_mmu *mmu, u16 address, u8 bit);
void unset_bit(gbc_mmu *mmu, u16 address, u8 bit);