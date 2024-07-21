#include "gt/gametank.h"
#include "gt/banking.h"
#include "gt/drawing_funcs.h"
#include "gt/input.h"
#include "gt/dynawave.h"
#include "gt/music.h"
#include "gt/feature/text/text.h"
#include "gen/assets/gfx.h"
#include "gen/assets/music.h"

extern int tanks_main();
extern int tetris_main();

char selection = 0;

char auto_tick_music = 0;

int main() {
    auto_tick_music = 0;
    init_graphics();
    init_dynawave();
    init_music();

    load_spritesheet(&ASSET__gfx__select_bmp, 0);

    clear_border(32);
    draw_sprite(1, 7, 126, 114, 0, 0, 0);
    draw_sprite(32, 60, 11, 9, 0, 119, 0);
    await_draw_queue();
    flip_pages();
    clear_border(32);
    draw_sprite(1, 7, 126, 114, 0, 0, 0);
    draw_sprite(9, 93, 11, 9, 0, 119, 0);
    await_draw_queue();
    auto_tick_music = 1;
    while (!(selection & 2))
    {
        update_inputs();
        if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_UP | INPUT_MASK_DOWN)) {
            flip_pages();
            selection = !selection;
            play_sound_effect(&ASSET__music__ping1_bin, 0);
        }
        if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_START | INPUT_MASK_A)) {
            selection |= 2;
        }
        sleep(1);
    }
    play_song(&ASSET__music__loading_mid, REPEAT_LOOP);
    draw_box(1, 7, 126, 114, 32);
    draw_sprite(32, 48, 64, 14, 16, 114, 0);
    await_draw_queue();
    flip_pages();
    draw_box(1, 7, 126, 114, 32);
    draw_sprite(32, 48, 64, 14, 16, 114, 0);
    await_draw_queue();
    flip_pages();
    //auto_tick_music = 0;
    if(selection & 1) {
        tetris_main();
    } else {
        tanks_main();
    }
    return 0;
}