#include "gametank.h"
#include "drawing_funcs.h"

char global_tick;
char game_state;
char state_timer;
char field[256];
char lives;
int score;
char field_offset_x, field_offset_y;

static char r,c,i;

char delta(char a, char b) {
    if(a > b) return a - b;
    return b - a;
}

void clear_field() {
    field_offset_x = 0;
    field_offset_y = 0;
    for(i = 0; i < 128; ++i) {
        field[i] = 0;
        field[i | 128] = 0;
    }
}

void draw_field(char tile_bank) {
    i = 0;
    *dma_flags = (flagsMirror | DMA_GCARRY) & ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    banksMirror = bankflip | GRAM_PAGE(tile_bank);
    *bank_reg = banksMirror;
    vram[WIDTH] = 8;
    vram[HEIGHT] = 8;
    for(r = field_offset_y; r < 128; r+=8) {
        vram[VY] = r;
        i = (r << 1);
        for(c = field_offset_x; c < 128; c+=8) {
            if(field[i]) {
                vram[VX] = c;
                vram[GX] = (field[i] & 0x0F) << 3;
                vram[GY] = (field[i] & 0xF0) >> 1;
                vram[START] = 1;
                wait();
            }
            ++i;
        }
    }
}