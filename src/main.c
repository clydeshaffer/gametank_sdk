#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/input.h"
#include "gt/gfx/sprites.h"

#include "gen/assets/sprites.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;
char t = 0, tpl, tmi, ti;
static char i;
char scaleX, scaleY;

extern char SineTheta;
extern char SineOffset;
signed char get_scaled_sine();

extern unsigned char Sine256[256];

SpriteSlot ballSpr;

void set_box() {
    SineTheta = ti;
    SineOffset = scaleX;
    box_x = get_scaled_sine() + 64;
    SineTheta = ti+64;
    SineOffset = scaleY;
    box_y = get_scaled_sine() + 64;
}

void main () {

    ballSpr = allocate_sprite(&ASSET__sprites__spikeball_bmp_load_list);
    set_sprite_frametable(ballSpr, &ASSET__sprites__spikeball_json);

    scaleX = 64;
    scaleY = 64;

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
            //tpl = ti + scale;
            //tmi = ti - scale;
            //box_x = ((Sine256[tpl] + Sine256[tmi]) >> 2) + 64;//get_scaled_sine(64, t) + 64;
            //box_y = ((Sine256[(tpl+64)%256] + Sine256[(tmi+64)%256]) >> 2) + 64;//get_scaled_sine(64, t) + 64;
            set_box();
            //box_y = 64; //get_scaled_sine(64, t) + 64;
            //queue_draw_box(box_x-4, box_y-4, 8, 8, 60);
            queue_draw_sprite_frame(ballSpr, box_x, box_y, ((t >> 3) + i) & 15, 0);
        }
 
        queue_clear_border(0);
        await_draw_queue();
        await_vsync(1);
        flip_pages();

        if(player1_buttons & INPUT_MASK_LEFT) ++scaleX;
        if(player1_buttons & INPUT_MASK_RIGHT) --scaleX;
        if(player1_buttons & INPUT_MASK_UP) ++scaleY;
        if(player1_buttons & INPUT_MASK_DOWN) --scaleY;
        //if(t & 1) ++scale;

        ++t;
    }
}