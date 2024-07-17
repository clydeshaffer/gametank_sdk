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

#define btn_check(a) \
        btn = player##a##_buttons;\
        if(btn & INPUT_MASK_UP) {\
            --y##a;\
        }\
        if(btn & INPUT_MASK_DOWN) {\
            ++y##a;\
        }\
        if(btn & INPUT_MASK_LEFT) {\
            --x##a;\
        }\
        if(btn & INPUT_MASK_RIGHT) {\
            ++x##a;\
        }\
        if(btn & INPUT_MASK_A) {\
            c##a = 28;\
        }\
\
        if(btn & INPUT_MASK_B) {\
            c##a = 124;\
        }\
\
        if(btn & INPUT_MASK_C) {\
            c##a = 220;\
        }\
\
        if(btn & INPUT_MASK_START) {\
            c##a = 7;\
        }\

int main () {
    static unsigned char frame = 0, subframe = 0, x1 = 64, y1 = 32, c1 = 92, x2 = 64, y2 = 64, c2 = 92;

    init_graphics();
    init_dynawave();
    init_music();

    clear_border(0);
    await_draw_queue();
    flip_pages();
    clear_border(0);
    await_draw_queue();

    load_big_spritesheet(&ASSET__character__jason, 1);

    play_song(&ASSET__music__badapple_nointro_mid, REPEAT_LOOP);

    via[DDRA] = 0xFF;

    while (1) {                                     //  Run forever
        ++count;
        via[ORA] = count;
        clear_screen(3);
        
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

        draw_box(x1 & 127, y1 & 127, 8, 8, c1);
        draw_box(x2 & 127, y2 & 127, 8, 8, c2);

        c1 = 92;
        c2 = 92;

        btn_check(1);

        btn_check(2);

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