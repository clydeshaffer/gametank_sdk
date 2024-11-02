#include "gt/sprites.h"
#include "gt/input.h"

#include "gen/assets/gfx.h"
#include "gen/assets/gfx/TemplateCharacter.json.h"

char player_x, player_y;
char player_frame, player_subframe;
char player_frame_start, player_frame_length;
char player_direction;

#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

SpriteSlot player_sprite;

void init_player() {
    player_x = 64;
    player_y = 64;
    player_direction = DIR_RIGHT;

    player_frame = 0;
    player_subframe = 0;

    player_frame_start = 0;
    player_frame_length = 0;

    player_sprite = allocate_sprite(&ASSET__gfx__TemplateCharacter_bmp_load_list);
    set_sprite_frametable(player_sprite, &ASSET__gfx__TemplateCharacter_json);
}

void draw_player() {
    draw_sprite_frame(player_sprite, player_x, player_y, player_frame_start + player_frame, 0);
}

void update_player() {
    static char moved_x, moved_y;
    moved_x = 0;
    moved_y = 0;

    /* fun tip: the versions of -- and ++ that 
        go before a variable instead of after are 
        slightly faster, because the original value is
        discarded */
    if(player1_buttons & INPUT_MASK_LEFT) {
        --moved_x;
        player_direction = DIR_LEFT;
    }
    if(player1_buttons & INPUT_MASK_RIGHT) {
        ++moved_x;
        player_direction = DIR_RIGHT;
    }
    if(player1_buttons & INPUT_MASK_UP) {
        --moved_y;
        player_direction = DIR_UP;
    }
    if(player1_buttons & INPUT_MASK_DOWN) {
        ++moved_y;
        player_direction = DIR_DOWN;
    }

    if(moved_x || moved_y) {
        player_x += moved_x;
        player_y += moved_y;
        
        switch (player_direction)
        {
            case DIR_UP:
                player_frame_start = TEMPLATECHARACTER_TAG_WALK_UP_START;
                player_frame_length = TEMPLATECHARACTER_TAG_WALK_UP_LENGTH;
                break;
            case DIR_RIGHT:
                player_frame_start = TEMPLATECHARACTER_TAG_WALK_RIGHT_START;
                player_frame_length = TEMPLATECHARACTER_TAG_WALK_RIGHT_LENGTH;
                break;
            case DIR_DOWN:
                player_frame_start = TEMPLATECHARACTER_TAG_WALK_DOWN_START;
                player_frame_length = TEMPLATECHARACTER_TAG_WALK_DOWN_LENGTH;
                break;
            case DIR_LEFT:
                player_frame_start = TEMPLATECHARACTER_TAG_WALK_LEFT_START;
                player_frame_length = TEMPLATECHARACTER_TAG_WALK_LEFT_LENGTH;
                break;
            default:
                break;
        }
    } else {
        switch (player_direction)
        {
            case DIR_UP:
                player_frame_start = TEMPLATECHARACTER_TAG_IDLE_UP_START;
                player_frame_length = TEMPLATECHARACTER_TAG_IDLE_UP_LENGTH;
                break;
            case DIR_RIGHT:
                player_frame_start = TEMPLATECHARACTER_TAG_IDLE_RIGHT_START;
                player_frame_length = TEMPLATECHARACTER_TAG_IDLE_RIGHT_LENGTH;
                break;
            case DIR_DOWN:
                player_frame_start = TEMPLATECHARACTER_TAG_IDLE_DOWN_START;
                player_frame_length = TEMPLATECHARACTER_TAG_IDLE_DOWN_LENGTH;
                break;
            case DIR_LEFT:
                player_frame_start = TEMPLATECHARACTER_TAG_IDLE_LEFT_START;
                player_frame_length = TEMPLATECHARACTER_TAG_IDLE_LEFT_LENGTH;
                break;
            default:
                break;
        }
    }

    ++player_subframe;
    if(player_subframe == 8) {
        player_subframe = 0;
        ++player_frame;
    }

    if(player_frame >= player_frame_length) {
        player_frame = 0;
        player_subframe = 0;
    }

}