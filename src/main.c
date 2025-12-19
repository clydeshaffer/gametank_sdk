#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/audio/music.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

void main () {

    init_music();

    load_instrument(0, get_instrument_ptr(0));
    load_instrument(1, get_instrument_ptr(1));
    load_instrument(2, get_instrument_ptr(2));
    load_instrument(3, get_instrument_ptr(0));

    while (1) {                                     //  Run forever
        queue_clear_screen(3);
        queue_clear_border(0);

        await_draw_queue();
        await_vsync(1);
        flip_pages();
        tick_music();
 
    }
}