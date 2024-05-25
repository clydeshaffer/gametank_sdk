#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/character.h"
#include "gen/assets/music.h"

#define JASON_FRAME_COUNT 68
int btn;
char count;

int main () {
    register unsigned char frame = 0, subframe = 0, x = 32, y = 32, c = 92;

    init_graphics();
    init_dynawave();
    init_music();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    load_big_spritesheet(&ASSET__character__jason, 1);

    play_song(&ASSET__music__badapple_nointro_mid, REPEAT_LOOP);

    via[DDRA] = 0xFF;

    while (1) {                                     //  Run forever
        ++count;
        via[ORA] = count;
        clear_screen(3);
        draw_box(x, y, 8, 8, c);
        draw_sprite_frame(
            &ASSET__character__jason_json,
            40, 32, frame,
            0, 1
        );

        draw_sprite_frame(
            &ASSET__character__jason_json,
            88, 50, frame,
            SPRITE_FLIP_X, 1
        );

        draw_sprite_frame(
            &ASSET__character__jason_json,
            50, 96, frame,
            SPRITE_FLIP_Y, 1
        );

        if(subframe & 1) {
            btn = player1_buttons;
        } else {
            btn = player2_buttons;
        }
        
        if(btn & INPUT_MASK_UP) {
            --y;
        }
        if(btn & INPUT_MASK_DOWN) {
            ++y;
        }
        if(btn & INPUT_MASK_LEFT) {
            --x;
        }
        if(btn & INPUT_MASK_RIGHT) {
            ++x;
        }

        if(btn & INPUT_MASK_A) {
            c = 28;
        }

        if(btn & INPUT_MASK_B) {
            c = 124;
        }

        if(btn & INPUT_MASK_C) {
            c = 220;
        }

        if(btn & INPUT_MASK_START) {
            c = 7;
        }

        ++subframe;
        if(subframe == 8) {
            ++frame;
            subframe = 0;
            if(frame == JASON_FRAME_COUNT) {
                frame = 0;
            }
        }

        await_draw_queue();
        sleep(1);
        flip_pages();
        update_inputs();
        tick_music();

    }

  return (0);                                     //  We should never get here!
}