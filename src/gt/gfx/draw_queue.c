#include "draw_queue.h"
#include "gametank.h"

void pushRect();

unsigned char queue_start = 0;
unsigned char queue_end = 0;
unsigned char queue_count = 0;
unsigned char queue_pending = 0;

void queue_draw_sprite_rect() {
    if(queue_count >= QUEUE_MAX) {
        asm("CLI");
        await_drawing();
    }

    asm("SEI");
    if(rect.b & SPRITE_OFFSET_X) { rect.gx |= 128; }
    if(rect.b & SPRITE_OFFSET_Y) { rect.gy |= 128; }
    rect.b = (rect.b & BANK_GRAM_MASK) | bankflip | CLIP_MODE_XY;
    queue_flags_param = DMA_GCARRY;
    pushRect();

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
}

void queue_draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c) {
    if(x > 127) {
        return;
    }
    if(y > 127) {
        return;
    }
    if(w == 0) {
        return;
    }
    if(h == 0) {
        return;
    }
    while(queue_count >= QUEUE_MAX) {
        asm("CLI");
        await_drawing();
    }
    if(x + w >= 128) {
        w = 128 - x;
    }
    if(y + h >= 128) {
        h = 128 - y;
    }
    asm("SEI");

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.gx = 0;
    rect.gy = 0;
    rect.b = bankflip;
    rect.c = ~c;
    queue_flags_param = DMA_COLORFILL_ENABLE | DMA_OPAQUE;
    pushRect();
    

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
}

void queue_clear_border(char c) {
    queue_draw_box(
        0,
        0,
        SCREEN_WIDTH-BORDER_RIGHT_WIDTH,
        BORDER_TOP_HEIGHT,
        c
    );
    queue_draw_box(
        0,
        BORDER_TOP_HEIGHT,
        BORDER_LEFT_WIDTH,
        SCREEN_HEIGHT-BORDER_TOP_HEIGHT,
        c
    );
    queue_draw_box(
        BORDER_LEFT_WIDTH,
        SCREEN_HEIGHT-BORDER_BOTTOM_HEIGHT,
        SCREEN_WIDTH-BORDER_LEFT_WIDTH,
        BORDER_BOTTOM_HEIGHT,
        c
    );
    queue_draw_box(
        SCREEN_WIDTH-BORDER_RIGHT_WIDTH,
        0,
        BORDER_RIGHT_WIDTH,
        SCREEN_HEIGHT-BORDER_BOTTOM_HEIGHT,
        c
    );
}

void queue_draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip) {
    while(queue_count >= QUEUE_MAX) {
        asm("CLI");
        await_drawing();
    }
    asm("SEI");
    queue_flags_param = DMA_GCARRY;
    sprite_fetch_frame(sprite, frame);
    rect.b = (sprite & BANK_GRAM_MASK) | bankflip | CLIP_MODE_XY;

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
    if(sprite & SPRITE_OFFSET_X) { rect.gx |= 128; }
    if(flip & SPRITE_FLIP_X) {
        rect.gx ^= 0xFF;
        rect.gx -= sprite_temp_frame.w - 1;
    }

    rect.gy = sprite_temp_frame.gy;
    if(sprite & SPRITE_OFFSET_Y) { rect.gy |= 128; }
    if(flip & SPRITE_FLIP_Y) {
        rect.gy ^= 0xFF;
        rect.gy -= sprite_temp_frame.h - 1;
    }

    rect.w = sprite_temp_frame.w | (flip & SPRITE_FLIP_X ? 128 : 0);
    rect.h = sprite_temp_frame.h | (flip & SPRITE_FLIP_Y ? 128 : 0);

    pushRect();

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
}

void queue_clear_screen(char c) {
    queue_draw_box(1, 7, SCREEN_WIDTH-2, SCREEN_HEIGHT - 15, c);
}

void await_draw_queue() {
    asm ("SEI");
    if(queue_pending != 0) {
        await_drawing();
    }
    while(queue_end != queue_start) {
        next_draw_queue();
        asm ("CLI");
        await_drawing();
    }
    vram[START] = 0;
    queue_pending = 0;
    asm ("CLI");
}