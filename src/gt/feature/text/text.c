#include "text.h"

#include "gametank.h"
#include "banking.h"
#include "../../gfx/draw_direct.h"
#include "../../../gen/assets/sdk_default.h"
#include "../../../gen/modules_enabled.h"

#define SPRITE_CHAR_W 8
#define SPRITE_CHAR_H 8
#define SPRITE_ROW_0_F 0x68
#define SPRITE_ROW_G_V 0x70
#define SPRITE_ROW_W_Z 0x78
#define SPRITE_CHAR_BLANK_X 0x70
#define SPRITE_CHAR_BLANK_Y 0x78

#define TEXT_CHAR_WIDTH 8
#define TEXT_CHAR_HEIGHT 8
#define TEXT_LINE_HEIGHT 8

#ifdef ENABLE_MODULE_TEXT

char font_slot;
char text_cursor_x, text_cursor_y;
char text_print_width, text_print_line_start;
unsigned char text_color;
char font_offset_x, font_offset_y;

void text_init() {
    text_cursor_x = 0;
    text_cursor_y = 0;
    text_print_width = 128;
    text_print_line_start = 0;
    text_color = TEXT_COLOR_BLACK;
}

SpriteSlot load_font() {
    font_slot = allocate_sprite(&ASSET__sdk_default__bios8_bmp_load_list);
    font_offset_x = SPRITE_OFFSET_X(font_slot);
    font_offset_y = SPRITE_OFFSET_Y(font_slot);
    return font_slot;
}

char text_tmp;
void text_print_string(char* str) {
    *dma_flags = (flagsMirror | DMA_GCARRY) & ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    banksMirror = bankflip | GRAM_PAGE(font_slot);
    *bank_reg = banksMirror;
    vram[WIDTH] = TEXT_CHAR_WIDTH;
    vram[HEIGHT] = TEXT_CHAR_HEIGHT;
    vram[VY] = text_cursor_y;
    while(*str != 0) {
        switch(*str) {
            case ' ':
                text_cursor_x += TEXT_CHAR_WIDTH;
                break;
            case '\n':
                text_cursor_y += TEXT_CHAR_HEIGHT;
                vram[VY] = text_cursor_y;
                break;
            case '\r':
                text_cursor_x = text_print_line_start;
                break;
            default:
                text_tmp = *str + text_color;
                if(text_cursor_x >= (text_print_width + text_print_line_start)) {
                    text_cursor_x -= text_print_width;
                    text_cursor_y += TEXT_CHAR_HEIGHT;
                    vram[VY] = text_cursor_y;
                }
                vram[VX] = text_cursor_x;
                vram[GX] = ((text_tmp & 0x0F) << 3) | font_offset_x;
                vram[GY] = (text_tmp & 0x70) | font_offset_y;
                vram[START] = 1;
                text_cursor_x += TEXT_CHAR_WIDTH;
                wait();
        }
        
        ++str;
    }
}

#pragma rodata-name (push, "PROG0")

const unsigned char decimal_conversion_table[100] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99
};

#pragma rodata-name (pop)

void text_sprint_num(char* s, unsigned char num) {
    if(num > 99) return;
    push_rom_bank();
change_rom_bank(0xFD);
    num = decimal_conversion_table[num];
    *s = (num >> 4) + '0';
    *(s+1) = (num & 0xF) + '0';
    pop_rom_bank();
}

#endif