#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/music.h"
#include "gen/assets/music2.h"

int main () {
    char col = 30, row = 20;
    int dx = 1, dy = 1;

    init_graphics();
    init_dynawave();
    init_music();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    play_song(&ASSET__music2__badapple_nointro_mid, REPEAT_LOOP);

    while (1) {                                     //  Run forever
        clear_screen(3);
        for(row = 0; row < 16; ++row) {
          draw_box(1, (row+4) << 2 , audio_amplitudes[row], 2, 92);
        }
        clear_border(0);
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        tick_music();
        
    }

  return (0);                                     //  We should never get here!
}