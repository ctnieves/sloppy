/*#include <gba_base.h>*/
/*#include <gba_systemcalls.h>*/
/*#include <gba_video.h>*/

#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "gbc_gpu.h"
#include "gbc_io.h"
#include "gbc_mmu.h"


void gpu_init(gbc_gpu *gpu) {
    gpu_start_frame(gpu);
}


void insertion_sort(void *array, int length,
		int compare(void*, int, int), void swap(void *, int, int)) {
	int i, j; // loop indexes

	for (j = 1; j < length; j++) {// index to insert array
		for (i = j-1; 0 <= i; i--) {
			// compare to previous index and swap if needed
			// else if previous index is smaller, this is my place
			if (!compare(array, i, i+1)) {
				swap(array, i, i+1);
			} else {
				i = 0;
			}
		}
	}
}

// Bit 7-6 - Shade for Color Number 3
// Bit 5-4 - Shade for Color Number 2
// Bit 3-2 - Shade for Color Number 1
// Bit 1-0 - Shade for Color Number 0
void set_palette(u8* p, u8 v)  {
	p[0] = v & 0x03;
	p[1] = (v & 0x0C) >> 2;
	p[2] = (v & 0x30) >> 4;
	p[3] = (v & 0xC0) >> 6;
}

u8 gpu_read_u8(gbc_gpu *gpu, u16 address) {
	switch (address) {
		case IO_CURLINE:
			return read_u8(gpu->mmu, IO_CURLINE);
		case IO_CMPLINE:
			return read_u8(gpu->mmu, IO_CMPLINE);
		case IO_BGRDPAL:
			return read_u8(gpu->mmu, IO_BGRDPAL);
		case IO_OBJ0PAL:
			return read_u8(gpu->mmu, IO_OBJ0PAL);
		case IO_OBJ1PAL:
			return read_u8(gpu->mmu, IO_OBJ1PAL);
		case IO_SCROLLY:
			return read_u8(gpu->mmu, IO_SCROLLY);
		case IO_SCROLLX:
			return read_u8(gpu->mmu, IO_SCROLLX);
		case IO_WNDPOSY:
			return read_u8(gpu->mmu, IO_WNDPOSX);
		case IO_WNDPOSX:
			return read_u8(gpu->mmu, IO_WNDPOSX);
		case IO_LCDSTAT:
			return read_u8(gpu->mmu, IO_LCDSTAT);
		default:
			return 0;
	}
}

void gpu_start_frame(gbc_gpu *gpu) {
	int i, j;


	// set win and obj to transparent
	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
            gpu->bg_disp[i * 256 + 1] = 0;
            gpu->win_disp[i * 256 + j] = TRANSPARENT;
            gpu->obj_disp[i * 256 + j] = TRANSPARENT;
		}
	}
	u8 r = gpu_run(gpu, 0);
}

// TODO: cleanup and optimize
void gpu_draw_line_fb(gbc_gpu *gpu, u8 line) {
    for (int i = 0; i < SIZE_X; i++) {
        int px_index = line * SIZE_X + i;
        gpu->fb[px_index] = gpu->bg_disp[line * 256 + i];
        // Add ordering between win and obj!
        // check non painted
        if (gpu->win_disp[line * 256 + i] < 8) {
            gpu->fb[px_index] = gpu->win_disp[line * 256 + i];
        }

        // check transparency
        if (gpu->obj_disp[(line + SPRITE_INI_Y) * 256 +
                i + SPRITE_INI_X] < 8) {
            if (gpu->fb[px_index] != 0
                    && gpu->obj_disp[(line + SPRITE_INI_Y) * 256 + i + SPRITE_INI_X] >= 4) {
                continue;
            }
            gpu->fb[px_index] = gpu->obj_disp[(line + SPRITE_INI_Y) * 256 + i + SPRITE_INI_X] & 0x03;
        }
    }

    /*printf("fb write\n");*/
	/*memcpy(gpu->fb, 0, sizeof gpu->fb);*/
}

void gpu_draw_line_bg(gbc_gpu *gpu, u8 line) {
	u16 bg_tile_map, tile_data;
	u8 oam_row, obj_line;
	u8 obj_line_a, obj_line_b;
	s16 obj;

	switch (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_BG_Tile_Map_Display_Select)) {
        case OPT_BG_Tile_Map_0:
            bg_tile_map = 0x9800;
            break;
        case OPT_BG_Tile_Map_1:
            bg_tile_map = 0x9C00;
            break;
	}

	switch (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_BGWindow_Tile_Data_Select)) {
        case OPT_BGWindow_Tile_Data_0:
            tile_data = 0x9000;
            break;
        case OPT_BGWindow_Tile_Data_1:
            tile_data = 0x8000;
            break;
	}

	// optimization for GBA. it's ARM CPU struggled with div and mod operators
	/*oam_row = (u8)Div(line + read_u8(gpu->mmu, IO_SCROLLY), 8);*/
	/*obj_line = (u8)DivMod(line + read_u8(gpu->mmu, IO_SCROLLY), 8);*/
    oam_row = (u8)((line + read_u8(gpu->mmu, IO_SCROLLY)) / 8);
    obj_line = (u8)((line + read_u8(gpu->mmu, IO_SCROLLY)) % 8);

	u8 i, j;
	for (i = 0; i < 32; i++) {
		if (tile_data == 0x9000) {
			obj = (s8)read_u8(gpu->mmu, bg_tile_map + oam_row * 32 + i);
		} else {
			obj = (u8)read_u8(gpu->mmu, bg_tile_map + oam_row * 32 + i);
		}
		obj_line_a = read_u8(gpu->mmu, tile_data + obj * 16 + obj_line * 2);
		obj_line_b = read_u8(gpu->mmu, tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			gpu->bg_disp[line * 256 + (u8)(i * 8 - read_u8(gpu->mmu, IO_SCROLLX) + j)] =
				gpu->bg_palette[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				    ((obj_line_b & (1 << (7 - j))) ? 2 : 0)
				];
		}
	}
}

void gpu_draw_line_win(gbc_gpu *gpu, u8 line) {
	u16 win_tile_map, tile_data;
	u8 oam_row, obj_line;
	u8 obj_line_a, obj_line_b;
	s16 obj;

	if (read_u8(gpu->mmu, IO_WNDPOSY)  > line || read_u8(gpu->mmu, IO_WNDPOSX) > SIZE_X) {
		return;
	}

	switch (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_Window_Tile_Map_Display_Select)) {
	    case OPT_Window_Tile_Map_0:
		    win_tile_map = 0x9800;
		    break;
	    case OPT_Window_Tile_Map_1:
		    win_tile_map = 0x9C00;
		    break;
	}

	switch (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_BGWindow_Tile_Data_Select)) {
	    case OPT_BGWindow_Tile_Data_0:
		    tile_data = 0x9000;
		    break;
	    case OPT_BGWindow_Tile_Data_1:
		    tile_data = 0x8000;
		    break;
	}

	// optimization for GBA. it's ARM CPU struggled with div and mod operators
	/*oam_row = Div((u8)(line - read_u8(gpu->mmu, IO_WNDPOSY)), 8);*/
	/*obj_line = DivMod((u8)(line - read_u8(gpu->mmu, IO_WNDPOSY)), 8);*/
	oam_row = (u8)((line - read_u8(gpu->mmu, IO_WNDPOSY)) / 8);
	obj_line = (u8)((line - read_u8(gpu->mmu, IO_WNDPOSY)) % 8);
	u8 i, j;
	for (i = 0; i < (SIZE_X - (read_u8(gpu->mmu, IO_WNDPOSX) - 8)) / 8 + 1; i++) {
		if (tile_data == 0x9000) {
			obj = (s8)read_u8(gpu->mmu, win_tile_map + oam_row * 32 + i);
		} else {
			obj = (u8)read_u8(gpu->mmu, win_tile_map + oam_row * 32 + i);
		}
		obj_line_a = read_u8(gpu->mmu, tile_data + obj * 16 + obj_line * 2);
		obj_line_b = read_u8(gpu->mmu, tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			gpu->win_disp[line * 256 + (u8)(i * 8 + read_u8(gpu->mmu, IO_WNDPOSX) - 7 + j)] =
				gpu->bg_palette[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				    ((obj_line_b & (1 << (7 - j))) ? 2 : 0)
				];
		}
	}
}

int obj_comp(void *array, int i, int j) {
	gpu_obj *a = ((gpu_obj**)array)[i];
	gpu_obj *b = ((gpu_obj**)array)[j];

	if (a->x < b->x) {
		return 0;
	}
	if (a->x > b->x) {
		return 1;
	}

	if (a->id < b->id) {
		return 0;
	}
	if (a->id > b->id) {
	    return 1;
	}
	// this should never be reached
	return 0;
}

void obj_swap(void *array, int i, int j) {
	gpu_obj *a = ((gpu_obj**)array)[i];
	gpu_obj *b = ((gpu_obj**)array)[j];

	((gpu_obj**)array)[i] = b;
	((gpu_obj**)array)[j] = a;
}

// 8x8 or 8x16
// Object Pattern Table = 8000 - 8FFF
// Object Atribute Table = FE00 - FE9F
// 10 sprites per scan line
// higher X -> draw first (lower X -> draw above others)
// same X -> FE00 highest, FE04 next highest
// Y = 0 or Y => 144+16, discard sprite
void gpu_draw_line_obj(gbc_gpu *gpu, u8 line) {
	int i, j, first;
	uint16_t addr, pos;
	u8 obj_height, objs_line_len, obj_line;
	u8 obj_line_a, obj_line_b, color;
	u8 x_flip, y_flip, behind;
	u8 *pal;
	gpu_obj objs[40];
	gpu_obj *objs_line[40];

	line += SPRITE_INI_Y;

	switch (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_OBJ_Size)) {
	    case OPT_OBJ_Size_8x8:
		    obj_height = 8;
		    break;
	    case OPT_OBJ_Size_8x16:
		    obj_height = 16;
		    break;
	}
	// Read all the obj attributes
	for (i = 0; i < 40; i++) {
		addr = MEM_OAM + i * 4;

		objs[i].id = i;
		objs[i].y = read_u8(gpu->mmu, addr++);
		objs[i].x = read_u8(gpu->mmu, addr++);
		objs[i].pat = read_u8(gpu->mmu, addr++);
		objs[i].flags = read_u8(gpu->mmu, addr);
	}

	// Take the candidate objects to be drawn in the line
	objs_line_len = 0;
	for (i = 0; i < 40; i++) {
		if((objs[i].y != 0) && (objs[i].y < SPRITE_END_Y) &&
				(objs[i].y <= line) && ((objs[i].y + obj_height) > line)) {
			objs_line[objs_line_len++] = &objs[i];

            printf("mode clk: %d\n", gpu->mode_clock);
            printf("Object %d:\n", objs[i].id);
            printf("x: %02X, y: %02X, pat: %02X\n", objs[i].x, objs[i].y, objs[i].pat);

            /*return;*/
		}
	}

	// Sort the candidate objects by priority
    insertion_sort(objs_line, objs_line_len, obj_comp, obj_swap);

	// Draw objects by order of priority
	if (objs_line_len > 10) {
		first = objs_line_len - 10;
	} else {
		first = 0;
	}

	for (i = first; i < objs_line_len; i++) {
		x_flip = (objs_line[i]->flags & OPT_OBJ_Flag_xflip) ? 1 : 0;
		y_flip = (objs_line[i]->flags & OPT_OBJ_Flag_yflip) ? 1 : 0;
		obj_line = (line - objs_line[i]->y) % obj_height;
		if (y_flip) {
			obj_line = obj_height - 1 - obj_line;
		}
		obj_line_a = read_u8(gpu->mmu, 0x8000 + objs_line[i]->pat * 16 +
				obj_line * 2);
		obj_line_b = read_u8(gpu->mmu, 0x8000 + objs_line[i]->pat * 16 +
				obj_line * 2 + 1);
		if (objs_line[i]->flags & OPT_OBJ_Flag_palette) {
			pal = gpu->obj1_palette;
		} else {
			pal = gpu->obj0_palette;
		}
		if (objs_line[i]->flags & OPT_OBJ_Flag_priority) {
			behind = 1;
		} else {
			behind = 0;
		}
		for (j = 0; j < 8; j++) {
			pos = line * 256 +
				(objs_line[i]->x + (x_flip ? 7 - j : j)) % 256;
			color = pal[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				    ((obj_line_b & (1 << (7 - j))) ? 2 : 0)
			];
			if (color < 8) {
				gpu->obj_disp[pos] = color;
				if (behind) {
					gpu->obj_disp[pos] |= 4;
				}
			}
		}
	}
}

void gpu_draw_line(gbc_gpu *gpu, u8 line) {
	if (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_BG_Display_Enable)) {
		gpu_draw_line_bg(gpu, line);
	}
	if (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_Window_Display_Enable)) {
		gpu_draw_line_win(gpu, line);
	}
    if (read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_OBJ_Display_Enable)) {
        gpu_draw_line_obj(gpu, line);
    }

    gpu_draw_line_fb(gpu, line);
}


/**
  FF41 - STAT - LCDC Status (R/W)
  Bit 6 - LYC=LY Coincidence Interrupt (1=Enable) (Read/Write)
  Bit 5 - Mode 2 OAM Interrupt         (1=Enable) (Read/Write)
  Bit 4 - Mode 1 V-Blank Interrupt     (1=Enable) (Read/Write)
  Bit 3 - Mode 0 H-Blank Interrupt     (1=Enable) (Read/Write)
  Bit 2 - Coincidence Flag  (0:LYC<>LY, 1:LYC=LY) (Read Only)
  Bit 1-0 - Mode Flag       (Mode 0-3, see below) (Read Only)
0: During H-Blank
1: During V-Blank
2: During Searching OAM-RAM
3: During Transfering Data to LCD Driver
The two lower STAT bits show the current status of the LCD controller.
Mode 0: The LCD controller is in the H-Blank period and
the CPU can access both the display RAM (8000h-9FFFh)
and OAM (FE00h-FE9Fh)
Mode 1: The LCD contoller is in the V-Blank period (or the
display is disabled) and the CPU can access both the
display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
Mode 2: The LCD controller is reading from OAM memory.
The CPU <cannot> access OAM memory (FE00h-FE9Fh)
during this period.
Mode 3: The LCD controller is reading from both OAM and VRAM,
The CPU <cannot> access OAM and VRAM during this period.
CGB Mode: Cannot access Palette Data (FF69,FF6B) either.
The following are typical when the display is enabled:
Mode 2  2_____2_____2_____2_____2_____2___________________2____
Mode 3  _33____33____33____33____33____33__________________3___
Mode 0  ___000___000___000___000___000___000________________000
Mode 1  ____________________________________11111111111111_____
The Mode Flag goes through the values 0, 2, and 3 at a cycle of about 109uS. 0 is present about 48.6uS, 2 about 19uS, and 3 about 41uS. This is interrupted every 16.6ms by the VBlank (1). The mode flag stays set at 1 for about 1.08 ms.
Mode 0 is present between 201-207 clks, 2 about 77-83 clks, and 3 about 169-175 clks. A complete cycle through these states takes 456 clks. VBlank lasts 4560 clks. A complete screen refresh occurs every 70224 clks.)
*/
u8 gpu_run(gbc_gpu *gpu, int cycles) {
    int byte_row_width = 160;
    if (!read_bit(gpu->mmu, IO_LCDCONT, MASK_LCDCONT_LCD_Display_Enable)) {
        gpu->reset = 1;
        return 0;
    }
	if (gpu->reset) {
		gpu->reset = 0;
        return 1;
	}

	gpu->mode_clock += cycles;

	if (gpu->mode_clock > LCD_LINE_CYCLES) {
	    gpu->mode_clock -= LCD_LINE_CYCLES;

		// LYC Update
		if (read_u8(gpu->mmu, IO_CURLINE) == read_u8(gpu->mmu, IO_CMPLINE)) {
			set_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_COINCIDENCE_FLAG);
			if (read_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_LYC_LY_COINCIDENCE_INTERRUPT)) {
				set_bit(gpu->mmu, IO_IFLAGS, MASK_INT_LCDSTAT_INT);
			}
		} else {
		    write_u8(gpu->mmu, IO_LCDSTAT, read_u8(gpu->mmu, IO_LCDSTAT) & 0xFB);
		}

	    // next line
	    write_u8(gpu->mmu, IO_CURLINE, (read_u8(gpu->mmu, IO_CURLINE) + 1) % LCD_VERT_LINES);

	    // VBLANK
	    if (read_u8(gpu->mmu, IO_CURLINE) == SIZE_Y) {
	        write_u8(gpu->mmu, IO_LCDSTAT, (read_u8(gpu->mmu, IO_LCDSTAT) & 0xF3) | 0x01);
	        // Set Mode Flag to VBLANK at LCDSTAT
	        unset_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_MODE_FLAG);
	        set_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_VBLANK);

	        // Interrupt VBlank
	        set_bit(gpu->mmu, IO_IFLAGS, MASK_INT_VBLANK);
	        if (read_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_MODE_1_VBLANK_INTERRUPT)) {
	            set_bit(gpu->mmu, IO_IFLAGS, MASK_INT_LCDSTAT_INT);
	        }

            SDL_Event e;
            SDL_PollEvent(&e);
            if (e.type == SDL_QUIT){
                gpu->quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                gpu->quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                gpu->quit = true;
            }
	    }
        // Normal line
	    else if (read_u8(gpu->mmu, IO_CURLINE) < SIZE_Y) {
            if (read_u8(gpu->mmu, IO_CURLINE) == 0) {
                // CLEAR SCREEN
                SDL_RenderClear(gpu->renderer);
            }
		    set_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_HBLANK);

            if (read_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_MODE_0_HBLANK_INTERRUPT)) {
			    set_bit(gpu->mmu, IO_IFLAGS, MASK_INT_LCDSTAT_INT);
            }
	    }
	}
    else if (read_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_HBLANK) && gpu->mode_clock >= DUR_HBLANK) {
		set_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_OAM);

		if (read_bit(gpu->mmu, IO_LCDSTAT, MASK_LCDSTAT_MODE_2_OAM_INTERRUPT)) {
			set_bit(gpu->mmu, IO_IFLAGS, MASK_INT_LCDSTAT_INT);
		}
		/*gpu->mode_clock -= DUR_HBLANK;*/
    }
    else if ((read_u8(gpu->mmu, IO_LCDSTAT) & OPT_MODE_OAM) && gpu->mode_clock >= DUR_OAM) {
		set_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_OAM_VRAM);
		gpu_draw_line(gpu, read_u8(gpu->mmu, IO_CURLINE));
		/*gpu->mode_clock -= DUR_OAM;*/
    }
    else if ((read_u8(gpu->mmu, IO_LCDSTAT) & OPT_MODE_OAM_VRAM) && gpu->mode_clock >= DUR_OAM_VRAM) {
		set_bit(gpu->mmu, IO_LCDSTAT, OPT_MODE_OAM_VRAM);
		gpu_draw_line(gpu, read_u8(gpu->mmu, IO_CURLINE));

        for (int y = 0; y < SIZE_Y; y++) {
            for (int x = 0; x < SIZE_X; x++) {
                int px_index = y * SIZE_X + x;
                u8 color = gpu->fb[px_index];

                SDL_SetRenderDrawColor(gpu->renderer, color, color, color, 0xFF);
                SDL_RenderDrawPoint(gpu->renderer, x, y);
            }
        }

        SDL_RenderPresent(gpu->renderer);
		/*gpu->mode_clock -= DUR_OAM_VRAM;*/
    }
}

unsigned long gbcToRgb32(unsigned const bgr15) {
    unsigned long const r = bgr15       & 0x1F;
    unsigned long const g = bgr15 >>  5 & 0x1F;
    unsigned long const b = bgr15 >> 10 & 0x1F;

    return ((r * 13 + g * 2 + b) >> 1) << 16
        | (g * 3 + b) << 9
        | (r * 3 + g * 2 + b * 11) >> 1;
}

/*u16 rgb32ToRgb16(u32 rgb32) {*/
/*int red = red / 8;*/
/*int green = green / 4;*/
/*int blue = blue / 8;*/
/*return RGB8(red, green, blue);*/
/*}*/

/*u16 color_dmg_to_gba(unsigned const bgr15) {*/
/*return rgb32ToRgb16(gbcToRgb32(bgr15));*/
/*//return gbcToRgb32(bgr15);*/
/*}*/
