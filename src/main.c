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

    play_song(&ASSET__music__badapple_mid, REPEAT_LOOP);
    sleep(30);

    while (1) {                                     //  Run forever
        clear_screen(7);
        for(row = 0; row < 16; ++row) {
          draw_box(1, (((row) << 5) + (row>>1) + 12) & 0x7F, audio_amplitudes[row], 2, 16);
        }
        clear_border(0);
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        PROFILER_START(0);
        tick_music();
        PROFILER_END(0);
        
    }

  return (0);                                     //  We should never get here!
}