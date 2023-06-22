/*
 * GameTank-specific implementation of drawing functions
 */
#include <zlib.h>
#include "drawing_funcs.h"
#include "gametank.h"
#include "banking.h"

char cursorX, cursorY;

void sleep(int frames) {
    int i;
    for(i = 0; i < frames; i++) {
        frameflag = 1;
        while(frameflag) {}
    }
}

void flip_pages() {
    frameflip ^= DMA_PAGE_OUT;
    bankflip ^= BANK_SECOND_FRAMEBUFFER;
    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | DMA_OPAQUE | frameflip | DMA_GCARRY;
    *dma_flags = flagsMirror;
    *bank_reg = bankflip;
}

void load_spritesheet(char* spriteData, char srcBank, char ramBank) {
    char oldFlags = flagsMirror;
    char oldBanks = banksMirror;
    char bankNum = ramBank & 7;
    char xbit = (ramBank & 8) << 4;
    char ybit = (ramBank & 16) << 3;
    change_rom_bank(srcBank);
    flagsMirror = DMA_ENABLE;
    *dma_flags = flagsMirror;
    *vram_VX = 0;
    *vram_VY = 0;
    *vram_GX = xbit;
    *vram_GY = ybit;
    *vram_WIDTH = 1;
    *vram_HEIGHT = 1;
    *vram_START = 1;
    *vram_START = 0;

    flagsMirror = 0;
    *dma_flags = flagsMirror;
    banksMirror = bankflip | GRAM_PAGE(ramBank);
    *bank_reg = banksMirror;
    inflatemem(vram, spriteData);
    flagsMirror = oldFlags;
    banksMirror = oldBanks;
    *dma_flags = flagsMirror;
    *bank_reg = banksMirror;
}

void clear_spritebank(char bank) {
    char oldFlags = flagsMirror;
    char oldBanks = banksMirror;
    char bankNum = bank & 7;
    char xbit = (bank & 8) << 4;
    char ybit = (bank & 16) << 3;
    unsigned int i = 0;
    flagsMirror = DMA_ENABLE;
    *dma_flags = flagsMirror;
    *vram_VX = 0;
    *vram_VY = 0;
    *vram_GX = xbit;
    *vram_GY = ybit;
    *vram_WIDTH = 1;
    *vram_HEIGHT = 1;
    *vram_START = 1;
    *vram_START = 0;

    flagsMirror = 0;
    *dma_flags = flagsMirror;
    banksMirror = bankflip | GRAM_PAGE(bank);
    *bank_reg = banksMirror;
    for(i = 0; i < 16384; i++) {
        vram[i] = 0;
    }
    flagsMirror = oldFlags;
    banksMirror = oldBanks;
    *dma_flags = flagsMirror;
    *bank_reg = banksMirror;
}

unsigned char queue_start = 0;
unsigned char queue_end = 0;
unsigned char queue_count = 0;
unsigned char queue_pending = 0;
#define QUEUE_MAX 250
Frame temp_frame;

void pushRect();

void draw_sprite_frame(const Frame* sprite_table, char x, char y, char frame, char flip, char bank, char offset) {
    while(queue_count >= QUEUE_MAX) {
        asm("CLI");
        wait();
    }
    asm("SEI");
    queue_flags_param = DMA_GCARRY;
    temp_frame = sprite_table[frame];
    bank |= bankflip;
    rect.b = bank;

    if(flip & SPRITE_FLIP_X) {
        rect.x = (x - temp_frame.w - temp_frame.x - 1);
    } else {
        rect.x = (temp_frame.x + x);
    }

    rect.y = (temp_frame.y + y);

    rect.gx = temp_frame.gx;
    if(flip & SPRITE_FLIP_X) {
        rect.gx ^= 0xFF;
        rect.gx -= temp_frame.w - 1;
    }
    rect.gy = temp_frame.gy + offset;
    rect.w = temp_frame.w | (flip & SPRITE_FLIP_X ? 128 : 0);
    rect.h = temp_frame.h;

    pushRect();

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
}

void draw_sprite() {
    if(rect.x > 127) {
        return;
    }
    if(rect.y > 127) {
        return;
    }
    if(rect.w == 0) {
        return;
    }
    if(rect.h == 0) {
        return;
    }
    if(queue_count >= QUEUE_MAX) {
        asm("CLI");
        wait();
    }

    if(rect.x + rect.w >= 128) {
        rect.w = 128 - rect.x;
    }
    if(rect.y + rect.h >= 128) {
        rect.h = 128 - rect.y;
    }

   asm("SEI");
    pushRect();

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
}

void draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c) {
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
        wait();
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

void await_draw_queue() {
    asm ("SEI");
    if(queue_pending != 0) {
        wait();
    }
    while(queue_end != queue_start) {
        next_draw_queue();
        wait();
    }
    vram[START] = 0;
    queue_pending = 0;
    asm ("CLI");
}

void clear_border(char c) {
    draw_box(0, 0, SCREEN_WIDTH-1, 7, c);
    draw_box(0, 7, 1, SCREEN_HEIGHT-7, c);
    draw_box(1, SCREEN_HEIGHT-8, SCREEN_WIDTH-1, 8, c);
    draw_box(SCREEN_WIDTH-1, 0, 1, SCREEN_HEIGHT-8, c);
}

void clear_screen(char c) {
    *dma_flags = flagsMirror | DMA_COLORFILL_ENABLE | DMA_OPAQUE;
    queue_pending = 1;
    vram[VX] = 1;
    vram[VY] = 7;
    vram[GX] = 0;
    vram[GY] = 0;
    vram[WIDTH] = SCREEN_WIDTH-2;
    vram[HEIGHT] = SCREEN_HEIGHT-7-8;
    vram[COLOR] = ~c;
    vram[START] = 1;
    *dma_flags = flagsMirror;
}

void draw_box_now(char x, char y, char w, char h, char c) {
    *dma_flags = flagsMirror | DMA_COLORFILL_ENABLE | DMA_OPAQUE;
    vram[VX] = x;
    vram[VY] = y;
    vram[GX] = 0;
    vram[GY] = 0;
    vram[WIDTH] = w;
    vram[HEIGHT] = h;
    vram[COLOR] = ~c;
    vram[START] = 1;
    *dma_flags = flagsMirror;
}

void draw_sprite_now(char x, char y, char w, char h, char gx, char gy, char ramBank) {
    banksMirror = bankflip | ramBank;
    *bank_reg = banksMirror;
    if(x + w >= 128) {
        w = 128 - x;
    }
    if(y + h >= 128) {
        h = 128 - y;
    }
    vram[VX] = x;
    vram[VY] = y;
    vram[GX] = gx;
    vram[GY] = gy;
    vram[WIDTH] = w;
    vram[HEIGHT] = h;
    vram[START] = 1;
}

void draw_fade(unsigned char opacity) {
    char oldFlags = flagsMirror;
    char oldBanks = banksMirror;
    flagsMirror |= DMA_ENABLE;
    flagsMirror &= ~DMA_GCARRY;
    flagsMirror = flagsMirror & ~DMA_OPAQUE;
    flagsMirror &= ~DMA_COLORFILL_ENABLE;
    *dma_flags = flagsMirror;
    banksMirror = banksMirror & 0xF8;
    *bank_reg = banksMirror;
    draw_sprite_now(0, 0, 127, 127, opacity&0xF0, 64, 0);
    wait();
    flagsMirror = oldFlags;
    banksMirror = oldBanks;
    *dma_flags = flagsMirror;
    *bank_reg = banksMirror;
}

void printnum(int num) {
    vram[VX] = cursorX;
    vram[VY] = cursorY;
    vram[GY] = SPRITE_ROW_0_F;
    vram[WIDTH] = SPRITE_CHAR_W;
    vram[HEIGHT] = SPRITE_CHAR_H;
    if(num == 0) {
        vram[GX] = 0;
        vram[START] = 1;
        wait();
    } else {
        while(num != 0) {
            vram[GX] = (num % 10) << 3;
            vram[START] = 1;
            wait();
            cursorX -= 8;
            num = num / 10;
            vram[VX] = cursorX;
        }
    }
}

void print_hex_num(char num) {
    vram[VX] = cursorX;
    vram[VY] = cursorY;
    vram[GY] = SPRITE_ROW_0_F;
    vram[WIDTH] = SPRITE_CHAR_W;
    vram[HEIGHT] = SPRITE_CHAR_H;

    vram[GX] = (num & 0xF0) >> 1;
    vram[START] = 1;
    wait();
    cursorX += 8;
    vram[VX] = cursorX;
    vram[GX] = (num & 0x0F) << 3;
    vram[START] = 1;
    wait();
    cursorX += 8;
}

void print(char* str) {
    flagsMirror &= ~DMA_COLORFILL_ENABLE;
    *dma_flags = flagsMirror;
    vram[WIDTH] = SPRITE_CHAR_W;
    vram[HEIGHT] = SPRITE_CHAR_H;
    while(*str != 0) {
        if(*str >= '0' && *str <= '9') {
            vram[GX] = (*str - '0') << 3;
            vram[GY] = SPRITE_ROW_0_F;
        } else if(*str >= 'a' && *str <= 'f') {
            vram[GX] = ((*str - 'a') << 3) + 0x50;
            vram[GY] = SPRITE_ROW_0_F;
        } else if(*str >= 'g' && *str <= 'v') {
            vram[GX] = (*str - 'g') << 3;
            vram[GY] = SPRITE_ROW_G_V;
        } else if(*str >= 'w' && *str <= 'z') {
            vram[GX] = (*str - 'w') << 3;
            vram[GY] = SPRITE_ROW_W_Z;
        } else {
            vram[GX] = SPRITE_CHAR_BLANK_X;
            vram[GY] = SPRITE_CHAR_BLANK_Y;
        }
        if(*str == '\n') {
            cursorX = 0;
            cursorY += 8;
        } else {
            vram[VX] = cursorX;
            vram[VY] = cursorY;
            vram[START] = 1;
            wait();
            cursorX += 8;
        }
        str++;
        if(cursorX >= 128) {
            cursorX = 0;
            cursorY += 8;
        }
        if(cursorY >= 128) {
            cursorX = 0;
            cursorY = 0;
        }
    }
}