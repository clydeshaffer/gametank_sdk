#ifndef SPRITES_H
#define SPRITES_H
#include "gfx_sys.h"

#define SPRITE_SLOT_INVALID(x) (x & 0b11100000)

//Sprite sheets MUST be one of these
#define PAGE_MASK_SINGLE 1
#define PAGE_MASK_WIDE 3
#define PAGE_MASK_TALL 5
#define PAGE_MASK_FULL 15

#define SPRITE_OFFSET_X_MASK 8
#define SPRITE_OFFSET_Y_MASK 16
#define SPRITE_OFFSET_XY_MASK 24

#define SPRITE_OFFSET_X(slot) ((slot & SPRITE_OFFSET_X_MASK) ? 128 : 0)
#define SPRITE_OFFSET_Y(slot) ((slot & SPRITE_OFFSET_Y_MASK) ? 128 : 0)

#define CONSTRUCT_SPRITE_SLOT(slot, quadrant) ((slot) | (quadrant))

typedef char SpriteSlot;

typedef struct SpritePage {
    const unsigned char* data;
    char bank;
    const struct SpritePage* next;
} SpritePage;

extern Frame sprite_temp_frame;

void load_spritesheet(const unsigned char* spriteData, char srcBank, char ramBank);
void clear_spritebank(char bank);

void sprite_fetch_frame(SpriteSlot sprite, char frame);

SpriteSlot allocate_sprite(const SpritePage* sprite);
void set_sprite_frametable(SpriteSlot sprite, const Frame *frametable, char frametable_bank);
void free_sprite(SpriteSlot slot);

#endif