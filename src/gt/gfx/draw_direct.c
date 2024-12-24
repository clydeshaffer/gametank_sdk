#include "draw_direct.h"
#define MODULE_DRAWQUEUE_INTERNAL_USE
#include "draw_queue.h"
#include "gametank.h"


char direct_sprite_offset_x = 0;
char direct_sprite_offset_y = 0;

void direct_prepare_sprite_mode(SpriteSlot sprite) {
    await_draw_queue(); //Make sure we don't intersect with the queued system.
    await_drawing();
    flagsMirror |= DMA_ENABLE | DMA_IRQ | DMA_GCARRY;
    flagsMirror &= ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    *dma_flags = flagsMirror;
    banksMirror &= ~(BANK_RAM_MASK | BANK_SECOND_FRAMEBUFFER | BANK_GRAM_MASK);
    banksMirror |= bankflip | (sprite & BANK_GRAM_MASK) | BANK_CLIP_X | BANK_CLIP_Y;
    *bank_reg = banksMirror;
    direct_sprite_offset_x = SPRITE_OFFSET_X(sprite);
    direct_sprite_offset_y = SPRITE_OFFSET_Y(sprite);
}

void direct_quick_select_sprite(SpriteSlot sprite) {
    banksMirror &= ~BANK_GRAM_MASK;
    banksMirror |= sprite & BANK_GRAM_MASK;
    *bank_reg = banksMirror;
    direct_sprite_offset_x = SPRITE_OFFSET_X(sprite);
    direct_sprite_offset_y = SPRITE_OFFSET_Y(sprite);
}

void direct_prepare_box_mode() {
    await_draw_queue(); //Make sure we don't intersect with the queued system.
    await_drawing();
    flagsMirror |= DMA_ENABLE | DMA_OPAQUE | DMA_IRQ | DMA_COLORFILL_ENABLE | DMA_OPAQUE;
    *dma_flags = flagsMirror;
    banksMirror &= ~(BANK_RAM_MASK | BANK_SECOND_FRAMEBUFFER);
    banksMirror |= bankflip | BANK_CLIP_X | BANK_CLIP_Y;
    *bank_reg = banksMirror;
}

void direct_prepare_array_mode() {
    await_draw_queue();
    await_drawing();
    flagsMirror |= DMA_CPU_TO_VRAM;
    flagsMirror &= ~DMA_ENABLE;
    *dma_flags = flagsMirror;
}

void direct_prepare_sprite_ram_array_mode(SpriteSlot sprite) {
    await_draw_queue();
    await_drawing();

    flagsMirror |= DMA_ENABLE | DMA_IRQ | DMA_GCARRY | DMA_COLORFILL_ENABLE;
    flagsMirror &= ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    *dma_flags = flagsMirror;
    banksMirror &= ~(BANK_RAM_MASK | BANK_SECOND_FRAMEBUFFER | BANK_GRAM_MASK);
    banksMirror |= bankflip | (sprite & BANK_GRAM_MASK) | BANK_CLIP_X | BANK_CLIP_Y;
    *bank_reg = banksMirror;
    direct_sprite_offset_x = SPRITE_OFFSET_X(sprite);
    direct_sprite_offset_y = SPRITE_OFFSET_Y(sprite);

    DIRECT_SET_COLOR(0);
    DIRECT_DRAW_SPRITE(0, 0, 0, 0, 0, 0);

    flagsMirror &= ~DMA_ENABLE;
    *dma_flags = flagsMirror;
}

void direct_tiled_mode(bool enabled) {
    if(enabled) {
        flagsMirror &= ~DMA_GCARRY;
    } else {
        flagsMirror |= DMA_GCARRY;
    }
    *dma_flags = flagsMirror;
}

void direct_transparent_mode(bool enabled) {
    if(enabled) {
        flagsMirror &= ~DMA_OPAQUE;
    } else {
        flagsMirror |= DMA_OPAQUE;
    }
    *dma_flags = flagsMirror;
}

void direct_clip_mode(clip_mode_t mode) {
    banksMirror &= ~CLIP_MODE_XY;
    banksMirror |= mode;
}

void direct_draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip) {
    asm("SEI");
    sprite_fetch_frame(sprite, frame);

    if(flip & SPRITE_FLIP_X) {
        rect.x = (x - sprite_temp_frame.w - sprite_temp_frame.x - 1);
    } else {
        rect.x = (sprite_temp_frame.x + x);
    }

    if(flip & SPRITE_FLIP_Y) {
        rect.y = (y - sprite_temp_frame.h - sprite_temp_frame.y - 1);
    } else {
        rect.y = (sprite_temp_frame.y + y);
    }

    rect.gx = sprite_temp_frame.gx;
    if(sprite & SPRITE_OFFSET_X_MASK) { rect.gx |= 128; }
    if(flip & SPRITE_FLIP_X) {
        rect.gx ^= 0xFF;
        rect.gx -= sprite_temp_frame.w - 1;
    }

    rect.gy = sprite_temp_frame.gy;
    if(sprite & SPRITE_OFFSET_Y_MASK) { rect.gy |= 128; }
    if(flip & SPRITE_FLIP_Y) {
        rect.gy ^= 0xFF;
        rect.gy -= sprite_temp_frame.h - 1;
    }

    await_drawing();
    direct_quick_select_sprite(sprite);
    vram[VX] = rect.x;
    vram[VY] = rect.y;
    vram[GX] = rect.gx;
    vram[GY] = rect.gy;
    vram[WIDTH] = sprite_temp_frame.w | (flip & SPRITE_FLIP_X ? 128 : 0);
    vram[HEIGHT] = sprite_temp_frame.h | (flip & SPRITE_FLIP_Y ? 128 : 0);

    DIRECT_DRAW_START();
}