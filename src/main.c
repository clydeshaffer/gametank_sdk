#include "gametank.h"
#include "drawing_funcs.h"
#include "random.h"
#include "gen/assets/masks.h"

extern char draw_busy;
extern char SineTable[256];

int main () {
    unsigned char col = 30, row = 0, row2, i;
    int dx = 1, dy = 1;

    init_graphics();
    load_spritesheet(&ASSET__masks__firemask_bmp, 8);

    flip_pages();
    clear_border(0);
    clear_screen(1);
    await_draw_queue();
    flip_pages();
    clear_border(0);
    clear_screen(1);
    await_draw_queue();

    vram[GX] = 0;
    vram[GY] = 0;
    vram[WIDTH] = 1;
    vram[HEIGHT] = 1;
    vram[START] = 1;
    *dma_flags = 0;
    for(row = 0; row < 128; row++) {
        for(col = 0; col < 16; ++col) {
            vram[col + ((row & 0x7F) << 7)] = 1;
        }
        for(col = 112; col < 128; ++col) {
            vram[col + ((row & 0x7F) << 7)] = 1;
        }
    }

    *dma_flags = flagsMirror;

    vram[GX] = 0;
    vram[GY] = 200;
    vram[WIDTH] = 1;
    vram[HEIGHT] = 1;
    vram[START] = 1;
    *dma_flags = 0;
    for(row = 0; row < 128; row++) {
        for(col = 0; col < 16; ++col) {
            vram[col + ((row & 0x7F) << 7)] = 1;
        }
        for(col = 112; col < 128; ++col) {
            vram[col + ((row & 0x7F) << 7)] = 1;
        }
    }

    *dma_flags = flagsMirror;

    while (1) {                                     //  Run forever
        draw_sprite(0, 1, 127, 126, 0, row + 128, 0);
        draw_sprite(0, 1, 127, 126, 128, 0, 0);

        ++row;
        row2 = row - 48;
        
        col = row;
        while(draw_busy) {
            ++col;
            vram[VX] =( SineTable[col] >> 4) - 8;
        }

        await_draw_queue();
        sleep(1);
        flip_pages();
        vram[GX] = 0;
        vram[GY] = row;
        vram[WIDTH] = 1;
        vram[HEIGHT] = 1;
        vram[START] = 1;
        *dma_flags = 0;
        for(col = 16; col < 112; ++col) {
            vram[col + ((row & 0x7F) << 7)] &= 0b00011111;
            vram[col + ((row & 0x7F) << 7)] |= 48;

            vram[col + (((row2+(col&15)) & 0x7F) << 7)] &= 0b00011111;
            vram[col + (((row2+(col&15)) & 0x7F) << 7)] |= 112;
        }
        *dma_flags = flagsMirror;
        vram[GX] = 0;
        vram[GY] = row2;
        vram[WIDTH] = 1;
        vram[HEIGHT] = 1;
        vram[START] = 1;
        *dma_flags = 0;
        for(col = 16; col < 112; ++col) {
            vram[col + (((row2+(col&15)) & 0x7F) << 7)] &= 0b00011111;
            vram[col + (((row2+(col&15)) & 0x7F) << 7)] |= 112;
        }
        *dma_flags = flagsMirror;
        
    }

  return (0);                                     //  We should never get here!
}