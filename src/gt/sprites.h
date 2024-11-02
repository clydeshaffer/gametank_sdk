#ifndef SPRITES_H
#define SPRITES_H

#include "drawing_funcs.h"

#define SPRITE_SLOT_INVALID(x) (x & 0b00111000)

//Sprite sheets MUST be one of these
#define PAGE_MASK_SINGLE 1
#define PAGE_MASK_WIDE 3
#define PAGE_MASK_TALL 5
#define PAGE_MASK_FULL 15

typedef char SpriteSlot;

typedef struct SpritePage {
    void* data;
    char bank;
    void* next;
} SpritePage;

SpriteSlot allocate_sprite(SpritePage* sprite);
void set_sprite_frametable(SpriteSlot sprite, const Frame *frametable, char frametable_bank);
void free_sprite(SpriteSlot slot);

void draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip);


#endif