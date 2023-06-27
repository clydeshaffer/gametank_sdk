#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"

#include "gen/assets/gfx.h"

typedef struct {
    char lsb, msb;
} twobytes;

typedef union {
    unsigned int i;
    twobytes b;
} coordinate;

int sine_wave[256] = {
    128,131,134,137,141,144,147,150,153,156,159,162,165,168,171,174,
    177,180,183,186,188,191,194,196,199,202,204,207,209,212,214,216,
    219,221,223,225,227,229,231,233,234,236,238,239,241,242,244,245,
    246,247,249,250,250,251,252,253,254,254,255,255,255,256,256,256,
    256,256,256,256,255,255,255,254,254,253,252,251,250,250,249,247,
    246,245,244,242,241,239,238,236,234,233,231,229,227,225,223,221,
    219,216,214,212,209,207,204,202,199,196,194,191,188,186,183,180,
    177,174,171,168,165,162,159,156,153,150,147,144,141,137,134,131,
    128,125,122,119,115,112,109,106,103,100,97,94,91,88,85,82,
    79,76,73,70,68,65,62,60,57,54,52,49,47,44,42,40,
    37,35,33,31,29,27,25,23,22,20,18,17,15,14,12,11,
    10,9,7,6,6,5,4,3,2,2,1,1,1,0,0,0,
    0,0,0,0,1,1,1,2,2,3,4,5,6,6,7,9,
    10,11,12,14,15,17,18,20,22,23,25,27,29,31,33,35,
    37,40,42,44,47,49,52,54,57,60,62,65,68,70,73,76,
    79,82,85,88,91,94,97,100,103,106,109,112,115,119,122,125
};

int main () {
    char flippage;
    coordinate angle = {0};
    coordinate tank_x = {16384};
    coordinate tank_y = {16384};
    char angle_frame;

    init_graphics();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    load_spritesheet(&ASSET__gfx__green_tank_bmp, 0);

    while (1) {             
        update_inputs();
        clear_screen(3);

        flippage = 0;
        if(angle.b.msb <= 64) {
            angle_frame = angle.b.msb >> 3;
        } else if(angle.b.msb <= 128) {
            angle_frame = (128 - angle.b.msb) >> 3;
            flippage = SPRITE_FLIP_Y;
        } else if(angle.b.msb <= 192) {
            angle_frame = (angle.b.msb - 129) >> 3;
            flippage = SPRITE_FLIP_BOTH;
        } else {
            angle_frame = (256 - angle.b.msb) >> 3;
            flippage = SPRITE_FLIP_X;
        }

        draw_sprite_frame(&ASSET__gfx__green_tank_json,
        tank_x.b.msb, tank_y.b.msb, angle_frame, flippage, 0);

        if(player1_buttons & INPUT_MASK_LEFT) {
            angle.b.msb += 1;
        }

        if(player1_buttons & INPUT_MASK_RIGHT) {
            angle.b.msb -= 1;
        }

        if(player1_buttons & INPUT_MASK_UP) {
            tank_y.i -= (sine_wave[(angle.b.msb + 64) % 256] - 128);
            tank_x.i += (sine_wave[(angle.b.msb + 128) % 256] - 128);
        } else if (player1_buttons & INPUT_MASK_DOWN) {
            tank_y.i += (sine_wave[(angle.b.msb + 64) % 256] - 128);
            tank_x.i -= (sine_wave[(angle.b.msb + 128) % 256] - 128);
        }
        

        await_draw_queue();
        sleep(1);
        flip_pages();
        
    }

  return (0);                                     //  We should never get here!
}