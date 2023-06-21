#include "gametank.h"
#include "drawing_funcs.h"

#include "gen/assets/characters.h"

int main () {
    char col = 30, row = 20;
    int dx = 1, dy = 1;
    char tick_counter = 0;
    char character_frame;

    flip_pages();

    clear_border(0);

    flip_pages();

    clear_border(0);

    //Load char_blue_1 into the Asset RAM in slot zero
    load_spritesheet(&ASSET__characters__char_blue_1_bmp, 0);

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

        //We just want to cycle through eight frames of animation, and advance every four screen frames
        character_frame = (tick_counter / 4) % 8;


        draw_sprite_frame(
            &ASSET__characters__char_blue_1_json_ptr, //Pointer to the sprite clipping data
            32, 72, //Position on screen
            character_frame, //Which frame number from the sprite sheet
            SPRITE_FLIP_NONE, //Sprite flipping options
            0, //Draw from slot zero of Asset RAM
            0 //Vertical offset into Asset RAM, usually zero
        );
        ++tick_counter; //This will loop after 255 and we're ok with that
        

        //Some drawing operations add to a queue instead of drawing immediately
        //This function waits for all queued drawings to finish before continuing
        await_draw_queue();

        flip_pages();
        sleep(1);
    }

  return (0);                                     //  We should never get here!
}