#ifndef SPRITES_H
#define SPRITES_H
#include "gfx_sys.h"

#define SPRITE_SLOT_INVALID(x) (x & 0b00111000)

//Sprite sheets MUST be one of these
#define PAGE_MASK_SINGLE 1
#define PAGE_MASK_WIDE 3
#define PAGE_MASK_TALL 5
#define PAGE_MASK_FULL 15

#define SPRITE_OFFSET_X 8
#define SPRITE_OFFSET_Y 16
#define SPRITE_OFFSET_XY 24

typedef char SpriteSlot;

typedef struct SpritePage {
    void* data;
    char bank;
    void* next;
} SpritePage;

extern Frame sprite_temp_frame;

void load_spritesheet(char* spriteData, char srcBank, char ramBank);
void clear_spritebank(char bank);

void sprite_fetch_frame(SpriteSlot sprite, char frame);

SpriteSlot allocate_sprite(SpritePage* sprite);
void set_sprite_frametable(SpriteSlot sprite, const Frame *frametable, char frametable_bank);
void free_sprite(SpriteSlot slot);

#endif