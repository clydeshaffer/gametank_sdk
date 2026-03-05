#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/sprites.h"
#include "gen/assets/anim.h"
#include "gen/assets/anim/cat.json.h"
#include "gen/assets/sdk_default.h"
#include "gt/input.h"
#include "gt/audio/music.h"
#include "gen/assets/music.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;
char cat_subframe = 0, cat_frame = 0;
const char cat_frame_lengths[11] = {128, 16, 16, 16, 16, 64, 16, 16, 16, 16, 8};

char buttonIndex = 0;
int buttonMasks[8] = { INPUT_MASK_DOWN, INPUT_MASK_UP, INPUT_MASK_LEFT, INPUT_MASK_RIGHT, INPUT_MASK_START, INPUT_MASK_C, INPUT_MASK_A, INPUT_MASK_B };
char text_x = 64;

SpriteSlot cat;
SpriteSlot controller;
SpriteSlot buttons;
SpriteSlot hello;
SpriteSlot tiles;

#define FLIP_BOX_X 0
#define FLIP_BOX_Y 65

void main () {
    cat = allocate_sprite(&ASSET__anim__cat_bmp_load_list);
    set_sprite_frametable(cat, &ASSET__anim__cat_json);
    controller = allocate_sprite(&ASSET__anim__controller_bmp_load_list);
    set_sprite_frametable(controller, &ASSET__anim__controller_json);
    buttons = allocate_sprite(&ASSET__anim__buttons_bmp_load_list);
    set_sprite_frametable(buttons, &ASSET__anim__buttons_json);
    hello = allocate_sprite(&ASSET__anim__hello_bmp_load_list);
    set_sprite_frametable(hello, &ASSET__anim__hello_json);
    tiles = allocate_sprite(&ASSET__anim__tiles_bmp_load_list);
    via[DDRB] = 0xFF;

    play_song(&ASSET__music__vampire_mid, REPEAT_LOOP);

    while (1) {                                     //  Run forever
        //queue_clear_screen(3);
        sprite_flags_override = DMA_GCARRY;
        queue_draw_sprite(0, 0, 127, 127, 16, 0, tiles);

        queue_draw_sprite(FLIP_BOX_X, FLIP_BOX_Y, 16, 16, 0, 0, tiles);
        queue_draw_sprite(FLIP_BOX_X+16, FLIP_BOX_Y, 128|16, 16, 256-16, 0, tiles);
        queue_draw_sprite(FLIP_BOX_X, FLIP_BOX_Y+16, 16, 128|16, 0, 256-16, tiles);
        queue_draw_sprite(FLIP_BOX_X+16, FLIP_BOX_Y+16, 128|16, 128|16, 256-16, 256-16, tiles);

        //Text that transparency can be turned off
        // by drawing ERROR in transparent over red box surrounded
        queue_draw_box(128-27, 7, 27, 9, 90);
        sprite_flags_override = DMA_OPAQUE;
        queue_draw_sprite(128-27, 7, 27, 9, 35, 3, tiles);

        //Test that graphic wraps if not clip mode
        // by drawing ERROR and then attempting to draw OK over it from offscreen coordinates
        queue_draw_sprite(128-31, 12, 32, 16, 64, 0, tiles);
        clip_override = CLIP_MODE_XY;
        queue_draw_sprite(5-32, 12, 28, 16, 96, 0, tiles);

        //Test that graphic doesn't wrap if CLIP mode by drawing ERROR offscreen
        queue_draw_sprite(-32, 20, 32, 16, 64, 0, tiles);


        queue_draw_box(box_x, box_y, 8, 8, 62);
        ++cat_subframe;
        if(cat_subframe == cat_frame_lengths[cat_frame]) {
            cat_subframe = 0;
            ++cat_frame;
            if(cat_frame == CAT_SHEET_FRAMES) {
                cat_frame = 0;
            }
        }
        

        update_inputs();

        queue_draw_sprite_frame(controller, 80, 80, 0, 0);

        for(buttonIndex = 0; buttonIndex < 8; ++buttonIndex) {
            if(player2_buttons & buttonMasks[buttonIndex]) {
                queue_draw_sprite_frame(buttons, 80, 80, buttonIndex, 0);
            }
        }

        queue_draw_sprite_frame(controller, 48, 32, 0, 0);

        for(buttonIndex = 0; buttonIndex < 8; ++buttonIndex) {
            if(player1_buttons & buttonMasks[buttonIndex]) {
                queue_draw_sprite_frame(buttons, 48, 32, buttonIndex, 0);
            }
        }

        queue_draw_sprite_frame(cat, 96, 52, cat_frame, 0);
        queue_draw_sprite_frame(hello, ++text_x, 108, 0, 0);
        queue_draw_sprite_frame(hello, text_x+128, 108, 0, 0);
        queue_draw_sprite_frame(hello, 12, 81, 1, 0);
        queue_clear_border(0);

        
        
        box_x += dx;
        box_y += dy;
        if(box_x == 1) {
            dx = 1;
        } else if(box_x == 119) {
            dx = -1;
        }
        if(box_y == 8) {
            dy = 1;
        } else if(box_y == 112) {
            dy = -1;
        }
 
        await_draw_queue();
        await_vsync(1);
        tick_music();
        flip_pages();
        ++via[ORB];
    }
}