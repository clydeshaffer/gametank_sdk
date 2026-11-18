#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"

#include "gt/gfx/sprites.h"
#include "gt/gfx/draw_direct.h"
#include "gt/feature/bittext/bittext.h"

#include "gen/assets/blini.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

void main () {

    SpriteSlot cat = allocate_sprite(&ASSET__blini__blini_bmp_load_list);
    SpriteSlot text = get_sprite_slot(PAGE_MASK_SINGLE);
    direct_prepare_sprite_ram_array_mode(text);
    
    bittext_color = 32;
    bittext_bg = 7;

    bittext_set_x(0);
    bittext_set_y(0);
    bittext_write_str("Give me the blini");

    while (1) {                                     //  Run forever
        //queue_clear_screen(3);
        queue_draw_sprite(0, 0, 127, 127, 64+(box_x>>1), 64, cat);
        queue_draw_sprite(box_x, box_y, 68, 8, 0, 0, text);
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
}