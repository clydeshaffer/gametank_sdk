#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "dynawave.h"
#include "music.h"

#include "gen/assets/audio.h"

#define COLOR_BOUNCING 92
#define COLOR_PLAYER_CONTROLLED 220

#define BOX_NORMAL_SIZE 8

char box_x = 30, box_y = 20;
char box_dx = 1, box_dy = 1;
char box_size = BOX_NORMAL_SIZE;
char box_color = COLOR_BOUNCING;

#define GAME_MODE_BOUNCING 0
#define GAME_MODE_CONTROLLER 1

char game_mode = GAME_MODE_BOUNCING;

int main () {
    
    //Setup the graphics and audio systems
    init_graphics();
    init_dynawave();
    init_music();

    /*  ASSET__audio__title_mid is an automatically-generated reference
        to assets/audio/title.mid in the graphics folder, and provides
        both the memory location and ROM bank number to play_song */
    play_song(&ASSET__audio__title_mid, REPEAT_NONE);

    while (1) {                                     //  Run forever
        clear_screen(3);
        draw_box(box_x, box_y, box_size, box_size, box_color);
        clear_border(0);

        /* An example of how you can represent different
            states of your game, such as a title screen,
            the main gameplay mode, or a pause menu. */
        if(game_mode == GAME_MODE_BOUNCING) {
            /* Simple bouncing movement */
            box_x += box_dx;
            box_y += box_dy;
            if(box_x == 1) {
                box_dx = 1;
            } else if(box_x == 119) {
                box_dx = -1;
            }
            if(box_y == 8) {
                box_dy = 1;
            } else if(box_y == 112) {
                box_dy = -1;
            }

            /* Assuming direct control */
            if(player1_buttons & INPUT_MASK_ANY_DIRECTION) {
                game_mode = GAME_MODE_CONTROLLER;
                box_color = COLOR_PLAYER_CONTROLLED;
                play_song(&ASSET__audio__gameloop_mid, REPEAT_LOOP);
            }
        } else if(game_mode == GAME_MODE_CONTROLLER) {

            /* fun tip: the versions of -- and ++ that 
                go before a variable instead of after are 
                slightly faster, because the original value is
                discarded */
            if(player1_buttons & INPUT_MASK_LEFT) {
                --box_x;
            }
            if(player1_buttons & INPUT_MASK_RIGHT) {
                ++box_x;
            }
            if(player1_buttons & INPUT_MASK_UP) {
                --box_y;
            }
            if(player1_buttons & INPUT_MASK_DOWN) {
                ++box_y;
            }

            if(player1_new_buttons & INPUT_MASK_A) {
                play_sound_effect(&ASSET__audio__flap_bin, 0);
                box_size = 12;
            } else if(box_size > BOX_NORMAL_SIZE) {
                --box_size;
            }
        }

        /* Make sure the draw_queue is finished before flipping pages */
        await_draw_queue();

        /* sleep(1) will wait for the next start of frame
        instead of a whole frame, i.e. waiting for vsync*/
        sleep(1);
        /* there are two frame buffers. typically we draw on one while
         displaying the other, and then switch them during vsync */
        flip_pages();
        update_inputs();
        tick_music();
    }

  return (0);
}