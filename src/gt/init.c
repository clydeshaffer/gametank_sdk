#include "./gametank.h"
#include "./drawing_funcs.h"
#include "./music.h"
#include "../gen/assets/sdk_default.h"

#pragma code-name (push, "COMMON")
#pragma rodata-name (push, "COMMON")

//ASM defined in draw_logo.s
void draw_gametank_logo(char color);
static const char logo_colors[] = {0, 0, 25, 57, 90, 122, 155, 187, 220, 252};
static const char fadeout_colors[] = { 7, 6, 5, 4, 3, 2, 1, 32};
static char logo_state;

#define INTRO_BG_COLOR 32

#define DIRECT_DRAW_START() draw_busy = 1; \
    vram[START] = 1;

#define DIRECT_DRAW_COLOR(dst_x, dst_y, w, h, c) \
    vram[VX] = dst_x; \
    vram[VY] = dst_y; \
    vram[WIDTH] = w; \
    vram[HEIGHT] = h; \
    vram[COLOR] = ~(c); \
    DIRECT_DRAW_START();

void sdk_init() {
    init_graphics();
    logo_state = 0;
    while(++logo_state) {
        
        await_draw_queue(); //Make sure we don't intersect with the queued system.
        await_drawing();
        flagsMirror |= DMA_ENABLE | DMA_OPAQUE | DMA_IRQ | DMA_COLORFILL_ENABLE | DMA_OPAQUE;
        *dma_flags = flagsMirror;
        banksMirror &= ~(BANK_RAM_MASK | BANK_SECOND_FRAMEBUFFER);
        banksMirror |= bankflip | BANK_CLIP_X | BANK_CLIP_Y;
        *bank_reg = banksMirror;
        asm ("CLI");

        if(logo_state < 3) {
            DIRECT_DRAW_COLOR(127, 127, 1, 1, INTRO_BG_COLOR);
            DIRECT_DRAW_COLOR(0, 0, 127, 127, INTRO_BG_COLOR);
            if(logo_state == 2) {
                init_dynawave();
                init_music();
            }
            await_drawing();
            DIRECT_DRAW_COLOR(127, 0, 1, 127, INTRO_BG_COLOR);
            await_drawing();
            DIRECT_DRAW_COLOR(0, 127, 127, 1, INTRO_BG_COLOR);
            await_drawing();
        }
        
        if(logo_state < 120 + (sizeof(fadeout_colors)<<2)) {
            if(logo_state > 120) {
                draw_gametank_logo(~fadeout_colors[(logo_state - 120) >> 2]);
            } else if(logo_state < 40) {
                draw_gametank_logo(~logo_colors[logo_state >> 2]);
            } else {
                draw_gametank_logo(~7);
            }
        }
        flip_pages();
        sleep(1);
        if(logo_state == 3) {
            play_song(&ASSET__sdk_default__jingle_mid, REPEAT_NONE);
        }
        if(logo_state > 2) {
            tick_music();
        }
        if(logo_state == 180) logo_state = 255;
    }
    stop_music();
}

#pragma code-name (pop)
#pragma rodata-name (pop)