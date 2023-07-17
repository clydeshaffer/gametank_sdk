#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx6.h"
#include "../gen/assets/music.h"
#include "random.h"
#include "music.h"

static char i;

#define MAZE_OFFSET_X_PIX 4
#define MAZE_OFFSET_ROW 4

#define HALF_LEVEL_WIDTH 7
#define HALF_LEVEL_HEIGHT 10
#define HALF_LEVEL_COUNT 1
#define HALF_LEVEL_BYTES HALF_LEVEL_WIDTH*HALF_LEVEL_HEIGHT*HALF_LEVEL_COUNT

#define LEVEL_LEFT_SIDE 0
#define LEVEL_RIGHT_SIDE 8

static char r,c;

static char penguin_x[2];
static char penguin_y[2];
static char penguin_status[2];

static char half_levels[HALF_LEVEL_BYTES] = {
    00,00,00,00,00,00,00,
    00,15,00,15,00,15,00,
    15,15,15,15,15,15,00,
    00,00,00,00,00,00,00,
    00,15,15,15,15,15,15,
    00,00,00,00,00,00,00,
    15,15,15,15,15,15,00,
    00,00,00,00,00,00,00,
    00,15,15,15,15,15,15,
    00,00,00,00,00,00,00
};

static void load_half_level(char level_num, char side) {
    i = level_num * (HALF_LEVEL_WIDTH * HALF_LEVEL_HEIGHT);
    for(r = 0; r < HALF_LEVEL_HEIGHT; ++r) {
        for(c = 0; c < HALF_LEVEL_WIDTH; ++c) {
            field[((r+4) << 4) + c + side] = half_levels[i++];
        }
    }
}

static void init_penguins() {
    penguin_x[0] = MAZE_OFFSET_X_PIX + (6 * 8);
    penguin_x[1] = MAZE_OFFSET_X_PIX + (8 * 8);
    penguin_y[0] = (MAZE_OFFSET_ROW + 9) * 8;
    penguin_y[1] = penguin_y[0];
}

static void try_move_penguin(char pid, char dx, char dy) {
    penguin_x[pid] += dx;
    penguin_y[pid] += dy;
    if((penguin_x[pid] != 3) &&
    (penguin_x[pid] != 117) &&
    (penguin_y[pid] != 31) &&
    (penguin_y[pid] != 105)) {
        c = (penguin_x[pid] - MAZE_OFFSET_X_PIX) >> 3;
        r = (((penguin_y[pid]) >> 3));
        if(!(field[(r << 4) + c] & 8)) {
            c = (penguin_x[pid] + 7 - MAZE_OFFSET_X_PIX) >> 3;
            if(!(field[(r << 4) + c] & 8)) {
                r = (((penguin_y[pid] + 7) >> 3));
                if(!(field[(r << 4) + c] & 8)) {
                    c = (penguin_x[pid] - MAZE_OFFSET_X_PIX) >> 3;
                    if(!(field[(r << 4) + c] & 8)) {
                        return;
                    }
                }
            }
        }
    }
    penguin_x[pid] -= dx;
    penguin_y[pid] -= dy;
}

void run_penguins_game() {
    await_draw_queue();
    flip_pages();
    sleep(1);

    load_spritesheet(&ASSET__gfx6__penguins_bmp, 0);

    rnd_seed = 234;
    global_tick = 0;
    clear_field();
    field_offset_x = 4;
    load_half_level(0, LEVEL_LEFT_SIDE);
    load_half_level(0, LEVEL_RIGHT_SIDE);

    global_tick = 55;
    field[global_tick-1] = 208;
    field[global_tick] = 209;
    field[global_tick+1] = 210;
    global_tick += 16;
    field[global_tick] = 129;
    global_tick += 16;
    for(i = 0; i < 9; ++i) {
        field[global_tick] = 15;
        global_tick += 16;
    }
    field[global_tick-1] = 192;
    field[global_tick] = 193;
    field[global_tick+1] = 194;

    init_penguins();

    while(1) {
        PROFILER_START(0);
        update_inputs();
        draw_box(0, 0, 127, 127, 32);
        
        //Top
        draw_tile(4, 24, 120, 8, 96, 0, 0);
        //Bottom
        draw_tile(4, SCREEN_HEIGHT-16, 120, 16, 96, 0, 0);
        //Left
        draw_tile(0, 24, 4, 104, 96, 0, 0);
        //Right
        draw_tile(SCREEN_WIDTH-4, 24, 4, 104, 96, 0, 0);

        await_draw_queue();

        if(player1_buttons & INPUT_MASK_LEFT) {
            try_move_penguin(0, -1, 0);
            try_move_penguin(1, 1, 0);
        }
        if(player1_buttons & INPUT_MASK_RIGHT) {
            try_move_penguin(0, 1, 0);
            try_move_penguin(1, -1, 0);
        }
        if(player1_buttons & INPUT_MASK_UP) {
            try_move_penguin(0, 0, -1);
            try_move_penguin(1, 0, -1);
        }
        if(player1_buttons & INPUT_MASK_DOWN) {
            try_move_penguin(0, 0, 1);
            try_move_penguin(1, 0, 1);
        }

        if(((penguin_x[0] >> 3) == (penguin_x[1] >> 3)) &&
            ((penguin_y[0] >> 3) == (penguin_y[1] >> 3)) &&
            (((penguin_x[0] - MAZE_OFFSET_X_PIX) >> 3) == 7) &&
            (((penguin_y[0]) >> 3) == MAZE_OFFSET_ROW)) {
                field[71] = 128;
            }
        
        draw_field(0);
        draw_sprite_now(penguin_x[0], penguin_y[0], 8, 8, 8, 8, 0);
        wait();
        draw_sprite_now(penguin_x[1], penguin_y[1], 8, 8, 40, 8, 0);
        wait();
        PROFILER_END(0);
        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }
}