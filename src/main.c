#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/music.h"

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

    play_song(&ASSET__music__title_mid, REPEAT_NONE);

    while (1) {                                     //  Run forever
        clear_screen(3);
        draw_box(col, row, 8, 8, 92);
        col += dx;
        row += dy;
        if(col == 1) {
            dx = 1;
        } else if(col == 119) {
            dx = -1;
        }
        if(row == 8) {
            dy = 1;
        } else if(row == 112) {
            dy = -1;
        }
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        tick_music();
        
    }

  return (0);                                     //  We should never get here!
}