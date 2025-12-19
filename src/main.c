#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/sprites.h"
#include "gen/assets/sdk_default.h"

char t = 0, t2 = 0;
SpriteSlot logo;
SpriteSlot text;

extern char SineTheta;
extern char SineOffset;
signed char get_scaled_sine();

void main () {
    logo = allocate_sprite(&ASSET__sdk_default__crowdsupply_logo_bmp_load_list);
    text = allocate_sprite(&ASSET__sdk_default__text_bmp_load_list);
    while (1) {                                     //  Run forever
        queue_draw_sprite(0, 0, 127, 127, 0, 0, logo);
        
        ++t;
        if(t == 2) {
            t = 0;
            if(t2 < 192)
                ++ t2;
        }
        SineTheta = t2;
        SineOffset = 50;

        queue_draw_sprite(8, 55 + get_scaled_sine(), 111, 18, 0, 0, text);

        queue_clear_border(0);
 
        await_draw_queue();
        await_vsync(1);
        flip_pages();
 
    }
}