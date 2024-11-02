#include "sprites.h"
#include "gametank.h"
#include "drawing_funcs.h"
#include "banking.h"

/*
    For each sprite bank:
        HEADS mask of which quadrants contain the anchor of a sheet
        OCCUPIED mask of which quadrants are allocated
        HEADS:4 | OCCUPIED:4
*/
static char bank_occupied[8] = { 0, 0, 0, 0, 0, 0, 0, 0};

typedef union PointerUnion {
    void* ptr;
    struct {
        char lsb, msb;
    } b;
} PointerUnion;

#define SPRITE_SLOT_COUNT 32
static char frametable_L[SPRITE_SLOT_COUNT];
static char frametable_H[SPRITE_SLOT_COUNT];
static char frametable_B[SPRITE_SLOT_COUNT];

#define OFFSET_X 64
#define OFFSET_Y 128
#define OFFSET_XY 192

#define HEAD_CORNER_NW 16
#define HEAD_CORNER_NE 32
#define HEAD_CORNER_SW 64
#define HEAD_CORNER_SE 128

SpriteSlot allocate_sprite(SpritePage* sprite) {
    register SpritePage* current;
    register char mask, i, free_page;
    change_rom_bank(0xFD);
    mask = 0; i = 1;
    for(current = sprite; current != 0; current = current->next) {
        if(current->data != 0) {
            mask |= i;
        }
        i = i << 1;
    }
    free_page = 0xFF;
    for(i = 0; i < 8; ++i) {
        if(!(bank_occupied[i] & mask)) {
            bank_occupied[i] |= mask | HEAD_CORNER_NW;
            free_page = i;
            break;
        }
        if(mask == PAGE_MASK_FULL) {
            continue;
        }
        if(mask != PAGE_MASK_WIDE) {
            //if mask doesn't span horizontally
            if(!(bank_occupied[i] & (mask << 1))) {
                bank_occupied[i] |= ((mask << 1) & 0b1010) | HEAD_CORNER_NE;
                free_page = i | OFFSET_X;
                break;
            }
        }
        if(mask != PAGE_MASK_TALL) {
            //if mask doesn't span vertically
            if(!(bank_occupied[i] & (mask << 2))) {
                bank_occupied[i] |= ((mask << 2) & 0b1100) | HEAD_CORNER_SW;
                free_page = i | OFFSET_Y;
                break;
            }
        }
        if(mask == 1) {
            if(!(bank_occupied[i] & (mask << 3))) {
                bank_occupied[i] |= ((mask << 3) & 0b1000) | HEAD_CORNER_SE;
                free_page = i | OFFSET_XY;
                break;
            }
        }
    }

    if(SPRITE_SLOT_INVALID(free_page)) {
        pop_rom_bank();
        return 0xFF;
    }

    i = 1;
    mask = (free_page & OFFSET_XY) >> 3;
    for(current = sprite; current != 0; current = current->next) {
        if(current->data != 0) {
            load_spritesheet(current->data, current->bank, (free_page & 7) | mask);
        }
        i = i << 1;
        mask += 8;
    }
    pop_rom_bank();
    return free_page;
}

void set_sprite_frametable(SpriteSlot sprite, const Frame *frametable, char frametable_bank) {
    frametable_B[sprite] = frametable_bank;
    frametable_L[sprite] = (*((PointerUnion*)(&frametable))).b.lsb;
    frametable_H[sprite] = (*((PointerUnion*)(&frametable))).b.msb;
}

void free_sprite(SpriteSlot slot) {
    register char anchor, clear_mask, heads;
    anchor = slot & OFFSET_XY;
    heads = bank_occupied[slot & 7] >> 4;
    clear_mask = ~heads;
    switch(slot & OFFSET_XY) {
        case 0:
            clear_mask &= 0b1111;
            clear_mask |= 1;
            if((clear_mask & 0b0110) != 0b110) {
                clear_mask &= 7;
            }
            break;
        case OFFSET_X:
            clear_mask &= 0b1010;
            clear_mask |= 2;
            break;
        case OFFSET_Y:
            clear_mask &= 0b1100;
            clear_mask |= 4;
            break;
        case OFFSET_XY:
            clear_mask = 0b1000;
            break;
    }
    clear_mask |= (clear_mask << 4);
    bank_occupied[slot & 7] &= ~clear_mask;
}

Frame temp_frame;
static PointerUnion dsf_frametable;

void pushRect();

void draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip) {
    change_rom_bank(frametable_B[sprite]);
    while(queue_count >= QUEUE_MAX) {
        asm("CLI");
        await_drawing();
    }
    asm("SEI");
    queue_flags_param = DMA_GCARRY;
    dsf_frametable.b.lsb = frametable_L[sprite];
    dsf_frametable.b.msb = frametable_H[sprite];
    temp_frame = ((Frame*) dsf_frametable.ptr)[frame];
    rect.b = (sprite & (~BANK_RAM_MASK)) | bankflip;

    if(flip & SPRITE_FLIP_X) {
        rect.x = (x - temp_frame.w - temp_frame.x - 1);
    } else {
        rect.x = (temp_frame.x + x);
    }

    if(flip & SPRITE_FLIP_Y) {
        rect.y = (y - temp_frame.h - temp_frame.y - 1);
    } else {
        rect.y = (temp_frame.y + y);
    }

    rect.gx = temp_frame.gx;
    if(sprite & 64) { rect.gx |= 128; }
    if(flip & SPRITE_FLIP_X) {
        rect.gx ^= 0xFF;
        rect.gx -= temp_frame.w - 1;
    }

    rect.gy = temp_frame.gy;
    if(sprite & 128) { rect.gy |= 128; }
    if(flip & SPRITE_FLIP_Y) {
        rect.gy ^= 0xFF;
        rect.gy -= temp_frame.h - 1;
    }

    rect.w = temp_frame.w | (flip & SPRITE_FLIP_X ? 128 : 0);
    rect.h = temp_frame.h | (flip & SPRITE_FLIP_Y ? 128 : 0);

    pushRect();

    if(queue_pending == 0) {
        next_draw_queue();
    }
    asm("CLI");
    pop_rom_bank();
}