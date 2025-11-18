#include "sprites.h"
#include "gametank.h"
#include "banking.h"
#include <zlib.h>
#include "../../gen/bank_nums.h"

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

#define HEAD_CORNER_NW 16
#define HEAD_CORNER_NE 32
#define HEAD_CORNER_SW 64
#define HEAD_CORNER_SE 128

void load_spritesheet(const unsigned char* spriteData, char srcBank, char ramBank) {
    char oldFlags = flagsMirror;
    char oldBanks = banksMirror;
    char bankNum = ramBank & 7;
    char xbit = (ramBank & 8) << 4;
    char ybit = (ramBank & 16) << 3;
    push_rom_bank();
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
    pop_rom_bank();
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

SpriteSlot allocate_sprite(const SpritePage* sprite) {
    register const SpritePage* current;
    register char mask, i, free_page;
    push_rom_bank();
    change_rom_bank(BANK_LOADERS);
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
                free_page = i | SPRITE_OFFSET_X_MASK;
                break;
            }
        }
        if(mask != PAGE_MASK_TALL) {
            //if mask doesn't span vertically
            if(!(bank_occupied[i] & (mask << 2))) {
                bank_occupied[i] |= ((mask << 2) & 0b1100) | HEAD_CORNER_SW;
                free_page = i | SPRITE_OFFSET_Y_MASK;
                break;
            }
        }
        if(mask == 1) {
            if(!(bank_occupied[i] & (mask << 3))) {
                bank_occupied[i] |= ((mask << 3) & 0b1000) | HEAD_CORNER_SE;
                free_page = i | SPRITE_OFFSET_XY_MASK;
                break;
            }
        }
    }

    if(SPRITE_SLOT_INVALID(free_page)) {
        pop_rom_bank();
        return 0xFF;
    }

    i = 1;
    mask = (free_page & SPRITE_OFFSET_XY_MASK);
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
    anchor = slot & SPRITE_OFFSET_XY_MASK;
    heads = bank_occupied[slot & 7] >> 4;
    clear_mask = ~heads;
    switch(slot & SPRITE_OFFSET_XY_MASK) {
        case 0:
            clear_mask &= 0b1111;
            clear_mask |= 1;
            if((clear_mask & 0b0110) != 0b110) {
                clear_mask &= 7;
            }
            break;
        case SPRITE_OFFSET_X_MASK:
            clear_mask &= 0b1010;
            clear_mask |= 2;
            break;
        case SPRITE_OFFSET_Y_MASK:
            clear_mask &= 0b1100;
            clear_mask |= 4;
            break;
        case SPRITE_OFFSET_XY_MASK:
            clear_mask = 0b1000;
            break;
    }
    clear_mask |= (clear_mask << 4);
    bank_occupied[slot & 7] &= ~clear_mask;
}

Frame sprite_temp_frame;
static PointerUnion dsf_frametable;

void sprite_fetch_frame(SpriteSlot sprite, char frame) {
    push_rom_bank();
    change_rom_bank(frametable_B[sprite]);
    dsf_frametable.b.lsb = frametable_L[sprite];
    dsf_frametable.b.msb = frametable_H[sprite];
    sprite_temp_frame = ((Frame*) dsf_frametable.ptr)[frame];
    pop_rom_bank();
}

void pushRect();