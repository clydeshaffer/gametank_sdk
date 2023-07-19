#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx.h"
#include "random.h"
#include "music.h"
#include "banking.h"
#include "../gen/assets/music2.h"

#define SNAKE_STATE_TITLE 0
#define SNAKE_STATE_RUNNING 1
#define SNAKE_STATE_DYING 2

#define SNAKE_TILE_EGG 15

static char snake_head_pos, snake_head_dir;
static char snake_tail_pos, snake_tail_dir;
static char i, dir_buffer;

#define DIR_RIGHT 0
#define DIR_DOWN 1
#define DIR_LEFT 2
#define DIR_UP 3

static const char dir_offset[4] = {1, 16, 255, 240};

void rnd_egg() {
    i = rnd_range(16, 240);
    while(field[i] != 0) {
        ++i;
        if(i >= 240) i = 16;
    }
    field[i] = SNAKE_TILE_EGG;
}

#pragma codeseg(push, "PROG0")

void _init_snake_game() {
    game_state = SNAKE_STATE_TITLE;
    global_tick = 0;
    snake_head_pos = 136;
    snake_head_dir = 0;
    snake_tail_pos = 132;
    snake_tail_dir = 0;
    clear_field();
    for(i = 1; i < 15; ++i) {
        field[i + 16] = 30;
        field[i + 224] = 62;
    }
    for(i = 32; i < 224; i += 16) {
        field[i] = 45;
        field[i+15] = 47;
    }
    field[snake_head_pos] = 2;
    field[snake_tail_pos] = 7;
    field[16] = 29;
    field[31] = 31;
    field[224] = 61;
    field[239] = 63;
    dir_buffer = 255;
    for(i = snake_tail_pos + 1; i < snake_head_pos; ++ i) {
        field[i] = 10;
    }
    rnd_egg();
}

char _iterate_snake() {
    i = snake_head_dir;
    if((dir_buffer != 255) && ((dir_buffer & 16) != (snake_head_dir & 16))) {
        field[snake_head_pos] = 8 + dir_buffer + ((snake_head_dir & 32) >> 5);
        snake_head_dir = dir_buffer;
        dir_buffer = 255;
        
    } else {
        field[snake_head_pos] = 10 + snake_head_dir;
    }
    snake_head_pos += dir_offset[snake_head_dir >> 4];

    if(field[snake_head_pos] == 0) {
        field[snake_tail_pos] = 0;
        snake_tail_pos += dir_offset[snake_tail_dir >> 4];
        snake_tail_dir = field[snake_tail_pos] & 0xF0;
        field[snake_tail_pos] = 7 + snake_tail_dir;
    } else if(field[snake_head_pos] == SNAKE_TILE_EGG) {
        do_noise_effect(80, 40, 4);
        rnd_egg();
    } else {
        snake_head_pos -= dir_offset[snake_head_dir >> 4];
        snake_head_dir = i;
        field[snake_head_pos] = 2 + snake_head_dir;
        return 1;
    }

    field[snake_head_pos] = 2 + snake_head_dir;
    return 0;
}

static const char corner_reverse_dirs[8] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};


char _skeletize_snake() {
    if(snake_head_pos == snake_tail_pos) {
        if(field[snake_head_pos] & 128) {
            return 1;
        }
        field[snake_head_pos] += 192;
        return 0;
    }
    field[snake_head_pos] += 192;
    snake_head_pos += dir_offset[snake_head_dir >> 4];
    i = field[snake_head_pos] & 15;
    if((i == 8) || (i == 9)) {
        i = field[snake_head_pos];
        i = ((i & 0x30) >> 3) | (i & 1);
        i = corner_reverse_dirs[i];
        snake_head_dir = i << 4;
    }
    return 0;
}
#pragma codeseg(pop)

void init_snake_game() {
    change_rom_bank(0xFE);
    _init_snake_game();
}

char iterate_snake() {
    change_rom_bank(0xFE);
    return _iterate_snake();
}

char skeletize_snake() {
    change_rom_bank(0xFE);
    return _skeletize_snake();
}

void run_snake_game() {
    await_draw_queue();
    sleep(1);
    flip_pages();
    
    load_spritesheet(&ASSET__gfx__snake_bmp, 0);
    rnd_seed = 234;
    init_snake_game();

    while(1) {
        update_inputs();
        clear_screen(76);
        switch(game_state) {
            case SNAKE_STATE_TITLE:
                rnd();
                draw_sprite(15, 32, 98, 40, 0, 32, 0);
                await_draw_queue();
                if(global_tick & 32) {
                    text_cursor_x = 8;
                    text_cursor_y = 96;
                    text_print_line_start = 8;
                    text_print_width = 128;
                    print_text("Presss ssstart");
                }
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    game_state = SNAKE_STATE_RUNNING;
                    //play_song(&ASSET__music2__lems_mid, REPEAT_LOOP);
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

                if((global_tick & 7) == 7) {
                    if(iterate_snake()) {
                        game_state = SNAKE_STATE_DYING;
                        snake_head_dir = (snake_head_dir + 32) & 0x30;
                        stop_music();
                    }
                }

                await_draw_queue();
                draw_field(0);
                
                break;
            case SNAKE_STATE_DYING:
                if((global_tick & 7) == 7) {
                    if(skeletize_snake())
                        init_snake_game();
                    else
                        do_noise_effect(80, 250, 3);
                }
                await_draw_queue();
                draw_field(0);
                break;
        }
        
        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }
}