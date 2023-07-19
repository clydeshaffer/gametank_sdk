#include "text.h"

#include "gametank.h"
#include "drawing_funcs.h"
#include "../../../gen/assets/font.h"
#include "banking.h"


#define TEXT_CHAR_WIDTH 8
#define TEXT_CHAR_HEIGHT 8
#define TEXT_LINE_HEIGHT 8

static char font_slot;
static char i, k;
char text_cursor_x, text_cursor_y;
char text_print_width, text_print_line_start;
char text_use_alt_color;

void init_text() {
    text_cursor_x = 0;
    text_cursor_y = 0;
    text_print_width = 128;
    text_print_line_start = 0;
    text_use_alt_color = 0;
}

void load_font(char slot) {
    font_slot = slot;
    load_spritesheet(&ASSET__font__bios8_bmp, slot);
}

char text_tmp;
void print_text(char* text) {
    *dma_flags = (flagsMirror | DMA_GCARRY) & ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    banksMirror = bankflip | GRAM_PAGE(font_slot);
    *bank_reg = banksMirror;
    vram[WIDTH] = TEXT_CHAR_WIDTH;
    vram[HEIGHT] = TEXT_CHAR_HEIGHT;
    vram[VY] = text_cursor_y;
    if(text_use_alt_color) {
        text_use_alt_color = 128;
    }
    while(*text != 0) {
        text_tmp = *text + text_use_alt_color;
        switch(*text) {
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
                if(text_cursor_x >= (text_print_width + text_print_line_start)) {
                    text_cursor_x -= text_print_width;
                    text_cursor_y += TEXT_CHAR_HEIGHT;
                    vram[VY] = text_cursor_y;
                }
                vram[VX] = text_cursor_x;
                vram[GX] = (text_tmp & 0x0F) << 3;
                vram[GY] = ((text_tmp & 0xF0) >> 1);
                vram[START] = 1;
                text_cursor_x += TEXT_CHAR_WIDTH;
                wait();
        }
        
        ++text;
    }
}

#pragma codeseg(push, "PROG0")
static void _num_to_str(int num, char* buf, char len) {
    if(num) {
        i = num;
        k = 1;
        while(i > 0) {
            buf[len-k] = '0' + (i % 10);
            i /= 10;
            ++k;
        }
        for(i = 0; i < (k-1); ++i) {
            buf[i] = buf[(len-1)-i];
        }
        buf[k] = 0;
    } else {
        buf[0] = '0';
        buf[1] = 0;
    }
}
#pragma codeseg(pop)

void num_to_str(int num, char* buf, char len) {
    change_rom_bank(0xFE);
    _num_to_str(num, buf, len);
}