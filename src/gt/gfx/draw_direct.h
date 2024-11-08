#ifndef DRAW_DIRECT_H
#define DRAW_DIRECT_H
#include "gametank.h"
#include "gfx_sys.h"
#include "sprites.h"

typedef char clip_mode_t;



//Setup registers to draw sprites in direct mode
//Also checks for and awaits queued drawing operations
void direct_prepare_sprite_mode(SpriteSlot sprite);

//Both this and direct_prepare_sprite_mode select the sprite slot to use
//but this skips other register settings and ONLY selects the sprite slot
void direct_quick_select_sprite(SpriteSlot sprite);

//Setup registers to draw colored boxes in direct mode
//Also checks for and awaits queued drawing operations
void direct_prepare_box_mode();

//If enabled, sprite draws will repeat the same 16x16 grid from the source data
void direct_tiled_mode(bool enabled);

//If enabled, sprite draws will treat color 0 as transparent instead of black
void direct_transparent_mode(bool enabled);

//Set whether drawings will be clipped at screen edges or wrap around
void direct_clip_mode(clip_mode_t mode);

void direct_draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip);

#define DIRECT_DRAW_START() draw_busy = 1; \
    vram[START] = 1;

#define DIRECT_DRAW_CLEAR_IRQ() vram[START] = 0;

//If you do consecutive draws that will cause redundant register sets
// then you can skip the macro and just set the registers directly.
// eg. for tilemaps you'd only need to set WIDTH, HEIGHT once and
// set VY once per row
#define DIRECT_DRAW_SPRITE(dst_x, dst_y, w, h, src_gx, src_gy) \
    vram[VX] = dst_x; \
    vram[VY] = dst_y; \
    vram[GX] = src_gx; \
    vram[GY] = src_gy; \
    vram[WIDTH] = w; \
    vram[HEIGHT] = h; \
    DIRECT_DRAW_START();

#define DIRECT_DRAW_SPRITE(dst_x, dst_y, w, h, c) \
    vram[VX] = dst_x; \
    vram[VY] = dst_y; \
    vram[WIDTH] = w; \
    vram[HEIGHT] = h; \
    vram[COLOR] = ~(c); \
    DIRECT_DRAW_START();

#endif