#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

int main () {
 

    while (1) {                                     //  Run forever
        direct_prepare_box_mode();
        dx = 0;
        for(box_y = 0; box_y < 128; box_y += 4) {
            for(box_x = 0; box_x < 128; box_x += 8) {
                DIRECT_DRAW_COLOR(box_x, box_y, 8, 4, dx);
                ++dx;
                //await_drawing();
            }
            DIRECT_DRAW_COLOR(127, box_y, 1, 4, 0);
        }
        await_vsync(1);
        flip_pages();
 
    }
 
  return (0);                                     //  We should never get here!
}