#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

int main () {
 
    while (1) {                                     //  Run forever
        queue_clear_screen(3);
        queue_draw_box(box_x, box_y, 8, 8, 92);
        queue_clear_border(0);
        
        box_x += dx;
        box_y += dy;
        if(box_x == 1) {
            dx = 1;
        } else if(box_x == 119) {
            dx = -1;
        }
        if(box_y == 8) {
            dy = 1;
        } else if(box_y == 112) {
            dy = -1;
        }
 
        await_draw_queue();
        await_vsync(1);
        flip_pages();
 
    }
 
  return (0);                                     //  We should never get here!
}