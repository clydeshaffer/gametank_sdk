#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/input.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;
char t = 0, tpl, tmi, ti;
static char i;
char scale;

signed char get_scaled_sine(char scale, char theta);

extern signed char Sine256[256];

void main () {
    scale = 16;
    while (1) {                                     //  Run forever
        queue_clear_screen(3);
        update_inputs();
        
        
        //box_x += dx;
        //box_y += dy;
        /*if(box_x == 1) {
            dx = 1;
        } else if(box_x == 119) {
            dx = -1;
        }*/
        /*if(box_y == 8) {
            dy = 1;
        } else if(box_y == 112) {
            dy = -1;
        }*/

        for(i = 0; i < 16; ++i) {
            ti = t + (i << 4);
            tpl = ti + scale;
            tmi = ti - scale;
            box_x = ((Sine256[tpl] + Sine256[tmi]) >> 2) + 64;//get_scaled_sine(64, t) + 64;
            box_y = ((Sine256[(tpl+64)%256] + Sine256[(tmi+64)%256]) >> 2) + 64;//get_scaled_sine(64, t) + 64;
            //box_y = 64; //get_scaled_sine(64, t) + 64;
            queue_draw_box(box_x-4, box_y-4, 8, 8, 60);
        }
 
        queue_clear_border(0);
        await_draw_queue();
        await_vsync(1);
        flip_pages();


        if(player1_buttons & INPUT_MASK_UP) ++scale;
        if(player1_buttons & INPUT_MASK_DOWN) --scale;

        ++t;
    }
}