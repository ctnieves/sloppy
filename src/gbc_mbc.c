#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "gbc_io.h"
#include "gbc_mbc.h"

void gbc_mbc_init(gbc_mbc *mbc) {
    mbc->num_rom_banks = 0;
    mbc->num_ram_banks = 0;
    mbc->rom_size = 0;
    mbc->ram_size = 0;

    mbc->bank1 = 1;
    mbc->bank2 = 0;

    mbc->mode_select = 0;
    mbc->cart_ram = 0;
    mbc->ram_enable = 0;

    // 0134-0143 - Title
    memcpy(&mbc->cart.title, &mbc->rom[0x134], 0x10);
    printf("TITLE: %s\n", mbc->cart.title);
    // 0147 - Cartridge Type
    mbc->type = mbc->rom[0x147];
    printf("MBC TYPE: 0x%x\n", mbc->type);
    // 0148 - ROM Size
    mbc->rom_size = mbc->rom[0x148];
    mbc->num_rom_banks = mbc->rom_numbytes / CART_ROM_BANK_SIZE;
    printf("ROM SIZE: 0x%x  BANKS: %d\n", mbc->rom_size, mbc->num_rom_banks);
    // 0149 - RAM Size
    mbc->ram_size = mbc->rom[0x149];
    mbc->num_ram_banks = mbc->rom_numbytes / CART_ROM_BANK_SIZE;
    printf("RAM SIZE: 0x%x\n", mbc->ram_size);
}

u8 gbc_mbc_read_u8(gbc_mbc *mbc, u16 address) {
    switch (mbc->type) {
        case ROM_ONLY:
            return mbc->rom[address];
        case MBC1: case MBC1_RAM: case MBC1_RAM_BATTERY:
            return mbc1_read_u8(mbc, address);
        case MBC2:
            return mbc2_read_u8(mbc, address);
        case MBC3: case MBC3_RAM:
            return mbc3_read_u8(mbc, address);
        case MBC5: case MBC5_RAM:
            return mbc5_read_u8(mbc, address);
        case ROM_RAM: case ROM_RAM_BATTERY:
        default:
            printf("read unsupported MBC type %d\n", mbc->type);
            break;
    }
    return 0;
}

void gbc_mbc_write_u8(gbc_mbc *mbc, u16 address, u8 val) {
    switch (mbc->type) {
        case ROM_ONLY:
            break;
        case ROM_RAM: case ROM_RAM_BATTERY:
            break;
        case MBC1: case MBC1_RAM: case MBC1_RAM_BATTERY:
            mbc1_write_u8(mbc, address, val);
            break;
        case MBC2:
            mbc2_write_u8(mbc, address, val);
            break;
        case MBC3: case MBC3_RAM:
            mbc3_write_u8(mbc, address, val);
            break;
        case MBC5: case MBC5_RAM:
            mbc5_write_u8(mbc, address, val);
            break;
        default:
            printf("wrote unsupported MBC type %d\n", mbc->type);
            break;
    }
}

// MBC1 (max 2MByte ROM and/or 32KByte RAM)
u8 mbc1_read_u8(gbc_mbc *mbc, u16 address) {
    if (address < 0x4000) { // ROM Bank 00
        // When the 0x0000-0x3FFF address range is accessed, the effective bank
        // number depends on the MODE register. In MODE 0b0 the bank number is
        // always 0, but in MODE 0b1 it’s formed by shifting the BANK2 register
        // value left by 5 bits
        u8 upper_bits = mbc->mode_select ? (mbc->bank2 << 5) : 0;
        long haddr = ((upper_bits << 14) | (address & 0x7FFF)) % mbc->rom_numbytes;
        //if (mbc->mode_select) {
            //printf("m:%d bank1: 0x%x  bank2:0x%x  addr:0x%x\n", mbc->mode_select, mbc->bank1, mbc->bank2, address);
        //}
        //return mbc->rom[address];
        return mbc->rom[haddr];
    }
    if (address < 0x8000) { // ROM Bank 01-7F (Read Only)
        // When the 0x4000-0x7FFF addess range is accessed, the effective bank
        // number is always a combination of BANK1 and BANK2 register values.
        // If the cartridge ROM is smaller than 16 Mbit, there are less ROM
        // address pins to connect to and therefore some bank number bits are
        // ignored. For example, 4 Mbit ROMs only need a 5-bit bank number, so
        // the BANK2 register value is always ignored because those bits are
        // simply not connected to the ROM.
        u8 rom_bank = (mbc->bank2 << 5) | mbc->bank1;
        long haddr = (address + (CART_ROM_BANK_SIZE * (rom_bank - 1))) % mbc->rom_numbytes;
        return mbc->rom[haddr];
    }
    if (address < 0xC000) { // RAM Bank 00-03, if any (Read/Write)
        /* This area is used to address external RAM in the cartridge (if any). External
         * RAM is often battery buffered, allowing to store game positions or high
         * score tables, even if the gameboy is turned off, or if the cartridge is
         * removed from the gameboy. Available RAM sizes are: 2KByte (at A000-A7FF),
         * 8KByte (at A000-BFFF), and 32KByte (in form of four 8K banks at *A000-BFFF). */
        u8 upper_bits = mbc->mode_select ? (mbc->bank2 << 5) : 0;
        u16 haddr = (address & 0x1FFF) | upper_bits;
        printf("read mbc1 ram upper : %x\n", haddr);
        return mbc->ram[haddr];
    }
    return 0xFF;
}

void mbc1_write_u8(gbc_mbc *mbc, u16 address, u8 val) {
    if (address < 0x2000) { // RAM Enable (Write Only)
        // RAMG - gbctr.pdf
        // bit 7-4 Unimplemented: Ignored during writes
        // bit 3-0 RAMG<3:0>: RAM gate register
        // 0b1010= enable access to cartridge RAM
        // All other values disable access to cartridge RAM
        if (val == 0xA) {
            mbc->ram_enable = 0xA;
        } else {
            mbc->ram_enable = 0;
        }
    }
    else if (address < 0x4000) { // ROM Bank Number (Write Only)
        // BANK1 - gbctr.pdf
        // The 5-bit BANK1 register is used as the lower 5 bits of the ROM bank number
        // when the CPU accesses the 0x4000-0x7FFF memory area.
        //
        // MBC1 doesn’t allow the BANK1 register to contain zero (bit pattern 0b00000),
        // so the initial value at reset is 0b00001 and attempting to write 0b00000
        // will write 0b00001 instead. This makes it impossible to read banks 0x00,
        // 0x20, 0x40 and 0x60 from the 0x4000-0x7FFF memory area, because those bank
        // numbers have 0b00000 in the lower bits. Due to the zero value adjustment,
        // requesting any of these banks actually requests the next bank (e.g. 0x21
        // instead of 0x20).

        // bit 7-5 Unimplemented: Ignored during writes
        // bit 4-0 BANK1<4:0>: Bank register 1
        // Never contains the value 0b00000.
        // If 0b00000 is written, the resulting value will be 0b00001 instead.

        mbc->bank1 = (val & 0x1F);
		if(mbc->bank1 == 0) {
            mbc->bank1 = 1;
		}
    }
    else if (address < 0x6000) { // RAM Bank Number - or - Upper Bits of ROM Bank Number (Write Only)
        // BANK2 - gbctr.pdf
        // bit 7-2 Unimplemented: Ignored during writes
        // bit 1-0 BANK2<1:0>: Bank register 2
        //
        // The 2-bit BANK2 register can be used as the upper bits of the ROM
        // bank number, or as the 2-bit RAM bank number. Unlike BANK1, BANK2
        // doesn’t disallow zero, so all 2-bit values are possible.
        mbc->bank2 = (val & 0x3);
    }
    else if (address < 0x8000) { // ROM/RAM Mode Select (Write Only)
        // MODE - gbctr.pdf
        // bit 7-1 Unimplemented: Ignored during writes
        // bit 0 MODE: Mode register
        // 0b1 = BANK2 affects accesses to 0x0000-0x3FFF, 0x4000-0x7FFF, 0xA000-0xBFFF
        // 0b0 = BANK2 affects only accesses to 0x4000-0x7FFF
        // The MODE register determines how the BANK2 register value is used during memory accesses
        mbc->mode_select = (val & 0x1);
    }
    else if (address >= 0xA000 && address < 0xC000) { // RAM Bank 00-03, if any (Read/Write)
        /* This area is used to address external RAM in the cartridge (if any).
         * External RAM is often battery buffered, allowing to store game
         * positions or high score tables, even if the gameboy is turned off,
         * or if the cartridge is removed from the gameboy. Available RAM sizes
         * are: 2KByte (at A000-A7FF), 8KByte (at A000-BFFF), and 32KByte (in
         * form of four 8K banks at *A000-BFFF). */
        if (mbc->ram_enable) {
            /*mbc->ram[address - 0xA000 + CART_RAM_BANK_SIZE * mbc->ram_bank] = val;*/
            u8 upper_bits = mbc->mode_select ? (mbc->bank2 << 5) : 0;
            u16 haddr = (address & 0x1FFF) | upper_bits;
            printf("write mbc1 ram upper : %x  v:%x\n", haddr, val);
            mbc->ram[haddr] = val;
        }
    }
}

// MBC2 (max 256KByte ROM and 512x4 bits RAM)
u8 mbc2_read_u8(gbc_mbc *mbc, u16 address) {
    if (address <= 0x7FFF) { // ROM Bank 01-0F (Read Only)

    }
    if (address <= 0x9FFF) { // unmapped
        return 0x00;
    }
    if (address <= 0xA1FF) { // 512x4bits RAM, built-in into the MBC2 chip (Read/Write)

    }
    return 0x00;
}

void mbc2_write_u8(gbc_mbc *mbc, u16 address, u8 val) {
}


// MBC3 (max 2MByte ROM and/or 64KByte RAM and Timer)
u8 mbc3_read_u8(gbc_mbc *mbc, u16 address) {
    if (address <= 0x7FFF) { // ROM Bank 01-7F (Read Only)

    }
    if (address <= 0xBFFF) { // RAM Bank 00-03, if any (Read/Write)

    }
    if (address <= 0xBFFF) { // RTC Register 08-0C (Read/Write)

    }
    if (address <= 0xA1FF) { // 512x4bits RAM, built-in into the MBC2 chip (Read/Write)

    }
    return 0x00;
}

void mbc3_write_u8(gbc_mbc *mbc, u16 address, u8 val) {
}

u8 mbc5_read_u8(gbc_mbc *mbc, u16 address) {
    return 0x00;
}

void mbc5_write_u8(gbc_mbc *mbc, u16 address, u8 val) {
}
