#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "gbc_mmu.h"
#include "gbc_mbc.h"
#include "gbc_io.h"

// from https://gist.github.com/domnikl/af00cc154e3da1c5d965k
void hex_dump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

void gbc_mmu_init(gbc_mmu *mmu){
    mmu->in_bios = true;
    mmu->oam_access = true;
    mmu->vram_access = true;

    memset(mmu->bios, 0xFF, sizeof mmu->bios);
    //memset(mmu->rom,  0xFF, sizeof mmu->rom);
    memset(mmu->vram, 0xFF, sizeof mmu->vram);
    memset(mmu->wram, 0xFF, sizeof mmu->wram);
    memset(mmu->oam,  0xFF, sizeof mmu->oam);
    memset(mmu->io,   0x00, sizeof mmu->io);
    memset(mmu->hram, 0xFF, sizeof mmu->hram);
    memset(mmu->zram, 0xFF, sizeof mmu->zram);
}

u8 z = 0;// hacky
u8* get_address_ptr(gbc_mmu *mmu, u16 address) {
    if (address < 0xA000) {
        return &mmu->vram[address & 0x1FFF];
    }
    if (address < 0xC000) {
        return &mmu->eram[address & 0x1FFF];
    }
    if (address < 0xE000) {
        return &mmu->wram[address & 0x1FFF];
    }
    if (address < 0xF000) {
        return &mmu->wram[address & 0x1FFF]; // echo
    }
    if (address < 0xFEA0) {
        return &mmu->oam[address & 0xFF];
    }
    if (address < 0xFF00) { // Not Usable
        z = 0;
        return &z;
    }
    if (address < 0xFF80) {
        return &mmu->io[address & 0xFF];
    }
    if (address <= 0xFFFF) {
        return &mmu->hram[address & 0xFF];
    }
    z = 0;
    return &z;
}

//void gbc_load_rom(gbc_mmu *mmu, const void *src, size_t n) {
    //memcpy(mmu->rom, src, n);
//}

void gbc_load_rom_file(gbc_mmu *mmu, const char *fname) {
    FILE *infile = fopen(fname, "rb");
    if(infile == NULL) {
        printf("broken 1\n");
        fflush(stdout);
        exit(1);
    }

    // Get the number of bytes
    fseek(infile, 0L, SEEK_END);
    mmu->mbc->rom_numbytes = ftell(infile);

    // reset the file position indicator to the beginning of the file
    fseek(infile, 0L, SEEK_SET);

    // grab sufficient memory for the buffer to hold the text
    mmu->mbc->rom = (u8*)malloc(mmu->mbc->rom_numbytes);

    // memory error
    if(mmu->mbc->rom == NULL) {
        printf("broken\n");
        fflush(stdout);
        fclose(infile);
        return;
    }

    // copy all the text into the buffer
    fread(mmu->mbc->rom, sizeof(char), mmu->mbc->rom_numbytes, infile);
    gbc_mbc_init(mmu->mbc);
    fclose(infile);
}

u8 read_u8(gbc_mmu *mmu, u16 address) {
    if (address < 0x100 && mmu->in_bios) {
        return mmu->bios[address];
    }
    if (address < 0x8000) {
        return gbc_mbc_read_u8(mmu->mbc, address);
    }

    u8 lcd_mode = mmu->io[IO_LCDSTAT & 0xFF] & MASK_LCDSTAT_MODE_FLAG;
    if (!mmu->oam_access) {
        if (address >= 0xFE00 && address < 0xFEA0) { // OAM
            return 0xFF;
        }
    }
    if (!mmu->vram_access) {
        if (address >= 0x8000 && address < 0xA000) { // VRAM
            return 0xFF;
        }
    }
    if (address == IO_DIV) {
        return mmu->counter->div >> 8;
    }
    return *get_address_ptr(mmu, address);
}

void write_u8(gbc_mmu *mmu, u16 address, u8 val) {
    if (address < 0x8000) {
        return gbc_mbc_write_u8(mmu->mbc, address, val);
    }

    u8 *ptr = get_address_ptr(mmu, address);

    if (address == 0xFF50) {
        mmu->in_bios = false;
    }

    // https://gbdev.gg8.se/wiki/articles/Serial_Data_Transfer_(Link_Cable)#FF02_-_SC_-_Serial_Transfer_Control_.28R.2FW.29
    if (address == 0xFF02 && val & 0x81) {
        printf("%c", read_u8(mmu, 0xFF01));
        fflush(stdout);
    }

    u8 lcd_mode = mmu->io[IO_LCDSTAT & 0xFF] & MASK_LCDSTAT_MODE_FLAG;
    // OAM is not accessible in Mode 2 or 3
    if (!mmu->vram_access) {
        if (address >= 0x8000 && address < 0xA000) { // VRAM
            return;
        }
    }
    if (!mmu->oam_access) {
        if (address >= 0xFE00 && address < 0xFEA0) // OAM
            return;
    }

    switch (address) {
        case IO_IFLAGS:
            *ptr = (val | 0b11100000);
            break;
        case IO_LCDCONT:
            *ptr = val;

            // fix LY to 0 when LCD is off
            if((*ptr & MASK_LCDCONT_LCD_Display_Enable) == 0) {
                // ensure LCD is on during vblank
                if (!read_bit(mmu, IO_LCDSTAT, OPT_MODE_VBLANK)) {
                    *ptr |= MASK_LCDCONT_LCD_Display_Enable;
                    return;
                }
                write_u8(mmu, IO_LCDSTAT, (read_u8(mmu, IO_LCDSTAT) & ~0x03) | OPT_MODE_VBLANK);
                write_u8(mmu, IO_CURLINE, 0);
            }
            break;
        case IO_DMACONT:
            *ptr = (val << 8);
            for(u8 i = 0; i < sizeof mmu->oam; i++) {
                mmu->oam[i] = read_u8(mmu, *ptr + i);
            }
            break;
        case IO_DIV:
            /* - When writing to DIV, the whole counter is reseted, so the timer
             *    is also affected. */
            mmu->counter->tima = 0;
            //mmu->counter->div = 0;

            /* - When writing to DIV, if the current output is '1' and timer is
             *    enabled, as the new value after reseting DIV will be '0', the
             *    falling edge detector will detect a falling edge and TIMA will
             *    increase. */
            /*if (*ptr == 1 && (read_io(mmu, IO_TAC) & MASK_TAC_ENABLE)) {*/
                /*write_io(mmu, IO_TIMA, read_u8(mmu, IO_TIMA) + 1);*/
            /*}*/
            *ptr = 0x00;
            break;
        case IO_TAC: {
            /* When writing to TAC, if the previously selected multiplexer
             * input was '1' and the new input is '0', TIMA will increase too.
             * This doesn't happen when the timer is disabled, but it also
             * happens when disabling the timer (the same effect as writing to
             * DIV).
             * https://gbdev.gg8.se/wiki/articles/Timer_Obscure_Behaviour */
            u8 old_TAC = *ptr;
            u8 new_TAC = val;
            *ptr = val;

            bool glitch = false;
            u16 old_clocks = TAC_CYCLES[old_TAC & MASK_TAC_CYCLES];
            u16 new_clocks = TAC_CYCLES[new_TAC & MASK_TAC_CYCLES];
            u8 old_enable = old_TAC & MASK_TAC_ENABLE;
            u8 new_enable = new_TAC & MASK_TAC_ENABLE;

            if (old_enable == 0) {
                 //TODO:
                 //has a different behaviour in GBC (AGB and AGS seem to have
                 //strange behaviour even in the other statements). When
                 //enabling the timer and maintaining the same frequency it
                 //doesn't glitch. When disabling the timer it doesn't glitch
                 //either. When another change of value happens (so timer is
                 //enabled after the write), the behaviour depends on a race
                 //condition, so it cannot be predicted for every device. 
                glitch = 0;
            } else {
                if (new_enable == 0) {
                    glitch = (mmu->counter->div & (old_clocks/2)) != 0;
                } else {
                    glitch = ((mmu->counter->div & (old_clocks/2)) != 0) && ((mmu->counter->div & (new_clocks/2)) == 0);
                }
            }

            if (glitch) {
                write_u8(mmu, IO_TIMA, read_u8(mmu, IO_TIMA) + 1);
            }
        }
        break;
        case IO_TIMA:
            //If you write to TIMA during the cycle that TMA is being loaded to it [B], the write will be ignored and TMA value will be written to TIMA instead.
            //write_io(mmu, IO_TIMA, read_u8(mmu, IO_TIMA) + 1);
            (*ptr) = val;
            break;
        case IO_JOYPAD:     // READ ONLY
            break;
        default:
            *ptr = val;
            break;
    }
}

u16 read_u16(gbc_mmu *mmu, u16 address) {
    // swap bytes for little-endian
    u16 temp = read_u8(mmu, address);
    temp |= read_u8(mmu, address+1) << 8;
    return temp;
}

void write_u16(gbc_mmu *mmu, u16 address, u16 val) {
    // swap bits for little-endian
    write_u8(mmu, address, val & 0xFF);
    write_u8(mmu, address+1, (u8)(val >> 8));
}

bool read_bit(gbc_mmu *mmu, u16 address, u8 bit) {
    return ((read_u8(mmu, address) & bit) > 0);
}

void set_bit(gbc_mmu *mmu, u16 address, u8 bit) {
    if (address == IO_LCDSTAT) {
        if (bit & OPT_MODE_OAM) {
            mmu->oam_access = false;
            mmu->vram_access = true;
        } else if (bit & OPT_MODE_OAM_VRAM) {
            mmu->oam_access = false;
            mmu->vram_access = false;
        }
    }
    u8 v = read_u8(mmu, address);
    write_u8(mmu, address, v | bit);
}

void unset_bit(gbc_mmu *mmu, u16 address, u8 bit) {
    if (address == IO_LCDSTAT) {
        if (bit & MASK_LCDSTAT_MODE_FLAG) {
            mmu->oam_access = true;
            mmu->vram_access = true;
        }
    }
    u8 v = read_u8(mmu, address);
    write_u8(mmu, address, v & ~bit);
}
