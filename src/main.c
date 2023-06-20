#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/music.h"

int main () {
    char col = 30, row = 20;
    int dx = 1, dy = 1;

    flip_pages();

    clear_border(0);

    flip_pages();

    clear_border(0);

    init_dynawave(); //Music Example: Initialize the soundcard
    init_music(); //Music Example: Initialize the music player
    play_song(&ASSET__music__fanfare_mid, REPEAT_NONE); //Music Example: Start playing a song

    while (1) {                                     //  Run forever
        clear_screen(3);
        draw_box_now(col, row, 8, 8, 92);
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

        tick_music(); //Music Example: We must call this every frame to continue music playback
        
        flip_pages();
        sleep(1);
    }

  return (0);                                     //  We should never get here!
}