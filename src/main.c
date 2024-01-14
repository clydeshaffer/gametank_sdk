#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/music.h"
#include "gen/assets/music2.h"

int main () {
    char channel, op;
    char y;

    init_graphics();
    init_dynawave();
    init_music();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    play_song(&ASSET__music2__vampire_mid, REPEAT_LOOP);
    sleep(30);

    while (1) {                                     //  Run forever
        clear_screen(7);
        y = 16;
        for(channel = 0; channel < 4; ++channel) {
          for(op = 0; op < 4; ++op) {
            draw_box(1, y + (op << 2), audio_amplitudes[(op << 2) + channel], 2, 16);
          }
          y += 24;
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