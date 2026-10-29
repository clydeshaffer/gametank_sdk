#include "gt/gametank.h"
#include "gt/input.h"
#include "gt/audio/music.h"
#include "gt/gfx/draw_queue.h"
#include "gen/assets/music.h"
#include "gen/assets/sdk_default.h"
char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

void main () {
    play_song(&ASSET__music__rudeb_bin, REPEAT_LOOP);
    while (1) {                                     //  Run forever
        queue_clear_screen(3);
        queue_draw_box(box_x, box_y, 8, 8, 92);
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

        if(player1_new_buttons & INPUT_MASK_UP) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 0);
        }
 
        if(player1_new_buttons & INPUT_MASK_DOWN) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 1);
        }
        
        if(player1_new_buttons & INPUT_MASK_LEFT) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 2);
        }
        
        if(player1_new_buttons & INPUT_MASK_RIGHT) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 3);
        }

        if(player1_new_buttons & INPUT_MASK_START) {
            play_song(&ASSET__sdk_default__jingle_bin, REPEAT_RESUME);
        }

        tick_music();

        await_draw_queue();
        await_vsync(1);
        flip_pages();
        update_inputs();
        
    }
}