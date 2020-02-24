#include <gba_base.h>
#include <string.h>

#include "gbc_mmu.h"

// also from https://github.com/drhelius/Gearboy/blob/master/src/Memory.h lol
const u8 kInitialValuesForFFXX[256] = {
    0xCF, 0x00, 0x7E, 0xFF, 0xD3, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA, 0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
    0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
    0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
    0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
    0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
    0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
    0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
    0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00
};

const u8 kInitialValuesForColorFFXX[256] = {
    0xCF, 0x00, 0x7C, 0xFF, 0x44, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
    0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
    0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
    0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
    0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
    0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
};

void gbc_mmu_init(gbc_mmu *mmu){
    mmu->in_bios = true;
}

u8* get_address_ptr(gbc_mmu *mmu , u16 address) {
    switch(address & 0xF000){
        // BIOS (256b)/ROM0
        case 0x0000:
            if(mmu->in_bios && address < 0x0100) {
                    return mmu->bios[address];
            }
            return &mmu->rom0[address];

            // ROM0
        case 0x1000:
        case 0x2000:
        case 0x3000:
            return &mmu->rom0[address];

            // ROM1 (unbanked) (16k)
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            return &mmu->rom1[address];

            // VRAM (8k)
        case 0x8000:
        case 0x9000:
            return &mmu->vram[address & 0x1FFF];

            // External RAM (8k)
        case 0xA000:
        case 0xB000:
            return &mmu->vram[address & 0x1FFF];

            // Working RAM (8k)
        case 0xC000:
        case 0xD000:
            return &mmu->wram0[address & 0x1FFF];

            // Working RAM shadow
        case 0xE000:
            return &mmu->wram0[address & 0x1FFF];

            // Working RAM shadow, I/O, Zero-page RAM
        case 0xF000:
            switch(address & 0x0F00){
                // Working RAM shadow
                case 0x000: case 0x100: case 0x200: case 0x300: case 0x400:
                case 0x500: case 0x600: case 0x700: case 0x800: case 0x900:
                case 0xA00: case 0xB00: case 0xC00: case 0xD00:
                    return &mmu->wram0[address & 0x1FFF];

                    // Graphics: object attribute memory
                    // OAM is 160 bytes, remaining bytes read as 0
                case 0xE00:
                    if(address < 0xFEA0) {
                        /*return &GPU->oam[address & 0xFF];*/
                    }
                    return 0;

                    // Zero-page
                case 0xF00:
                    if(address >= 0xFF80) {
                        return &mmu->zram[address & 0x7F];
                    }

                    // I/O control handling
                    // Currently unhandled
                    return 0;
            }
        default:
            return 0;
    }
}

u8 read_u8(gbc_mmu *mmu , u16 address) {
    return *get_address_ptr(mmu, address);
}

void write_u8(gbc_mmu *mmu , u16 address, u8 val) {
    u8 *ptr = get_address_ptr(mmu, address);
    *ptr = val;
}

u16 read_u16(gbc_mmu *mmu , u16 address) {
    u8 *ptr = get_address_ptr(mmu, address);
    u16 val;
    memcpy(&val, *ptr, sizeof(u16));

    return val;
}

void write_u16(gbc_mmu *mmu , u16 address, u16 val) {
    u8 *ptr = get_address_ptr(mmu, address);
    memcpy(ptr, val, sizeof(u16));
}

