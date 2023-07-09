#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx.h"
#include "random.h"

#define SNAKE_STATE_TITLE 0
#define SNAKE_STATE_RUNNING 1
#define SNAKE_STATE_GAMEOVER 2

#define SNAKE_TILE_EGG 15

static char snake_head_pos, snake_head_dir;
static char snake_tail_pos, snake_tail_dir;
static char i, dir_buffer;

static const char dir_offset[4] = {1, 16, 255, 240};

void rnd_egg() {
    i = rnd_range(16, 240);
    while(field[i] != 0) {
        ++i;
        if(i >= 240) i = 16;
    }
    field[i] = SNAKE_TILE_EGG;
}

void init_snake_game() {
    game_state = SNAKE_STATE_TITLE;
    global_tick = 0;
    snake_head_pos = 136;
    snake_head_dir = 0;
    snake_tail_pos = 132;
    snake_tail_dir = 0;
    rnd_seed = 234;
    clear_field();
    field[snake_head_pos] = 2;
    field[snake_tail_pos] = 7;
    dir_buffer = 255;
    for(i = snake_tail_pos + 1; i < snake_head_pos; ++ i) {
        field[i] = 10;
    }
    rnd_egg();
}

void iterate_snake() {
    if((dir_buffer != 255) && ((dir_buffer & 16) != (snake_head_dir & 16))) {
        field[snake_head_pos] = 8 + dir_buffer + ((snake_head_dir & 32) >> 5);
        snake_head_dir = dir_buffer;
        dir_buffer = 255;
        
    } else {
        field[snake_head_pos] = 10 + snake_head_dir;
    }
    snake_head_pos += dir_offset[snake_head_dir >> 4];

    if(field[snake_head_pos] != SNAKE_TILE_EGG) {
        field[snake_tail_pos] = 0;
        snake_tail_pos += dir_offset[snake_tail_dir >> 4];
        snake_tail_dir = field[snake_tail_pos] & 0xF0;
        field[snake_tail_pos] = 7 + snake_tail_dir;
    } else {
        rnd_egg();
    }

    field[snake_head_pos] = 2 + snake_head_dir;
}

void run_snake_game() {

    load_spritesheet(&ASSET__gfx__snake_bmp, 0);
    init_snake_game();

    while(1) {
        update_inputs();
        clear_screen(76);
        switch(game_state) {
            case SNAKE_STATE_TITLE:
                draw_sprite(15, 32, 98, 40, 0, 32, 0);
                await_draw_queue();
                if(global_tick & 32) {
                    text_cursor_x = 8;
                    text_cursor_y = 96;
                    print_text("Presss ssstart");
                }
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    game_state = SNAKE_STATE_RUNNING;
                }
                break;
            case SNAKE_STATE_RUNNING:

                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
                    dir_buffer = 0;
                }
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_DOWN) {
                    dir_buffer = 16;
                }
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
                    dir_buffer = 32;
                }
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_UP) {
                    dir_buffer = 48;
                }

                if((global_tick & 15) == 15) {
                    iterate_snake();
                }

                await_draw_queue();
                draw_field(0);
                
                break;
        }
        
        sleep(1);
        flip_pages();
        ++global_tick;
    }
}