#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx6.h"
#include "../gen/assets/music.h"
#include "random.h"
#include "music.h"
#include "./penguin_levels.h"

static char i;


#define PENGUINS_GY 8
#define FACE_UP 0
#define FACE_DOWN 8
#define FACE_RIGHT 16
#define FACE_LEFT 24
#define PINK_PENGUIN 32

static char r,c;

static char penguin_x[2];
static char penguin_y[2];
static char penguin_status[2];
static char penguin_gx[2];
static char penguin_gy[2];

static void init_penguins() {
    penguin_x[0] = MAZE_OFFSET_X_PIX + (6 * 8);
    penguin_x[1] = MAZE_OFFSET_X_PIX + (8 * 8);
    penguin_y[0] = (MAZE_OFFSET_ROW + 9) * 8;
    penguin_y[1] = penguin_y[0];
    penguin_gx[0] = FACE_DOWN;
    penguin_gx[1] = FACE_DOWN + PINK_PENGUIN;
    penguin_gy[0] = 0;
    penguin_gy[1] = 0;
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

static void init_field() {
    clear_field();
    field_offset_x = 4;
    

    r = 55;
    field[r-1] = 208;
    field[r] = 209;
    field[r+1] = 210;
    r += 16;
    field[r] = 129;
    r += 16;
    for(i = 0; i < 9; ++i) {
        field[r] = 11 + ((level_num & 7) << 5);
        r += 16;
    }
    field[r-1] = 192;
    field[r] = 193;
    field[r+1] = 194;
    load_level_num();
}

void run_penguins_game() {
    await_draw_queue();
    flip_pages();
    sleep(1);

    load_spritesheet(&ASSET__gfx6__penguins_bmp, 0);

    rnd_seed = 234;
    global_tick = 0;
    level_num = 1;
    init_field();

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
            penguin_gx[0] = FACE_LEFT;
            penguin_gx[1] = FACE_RIGHT + PINK_PENGUIN;
            penguin_gy[0] = global_tick & 24;
            penguin_gy[1] = global_tick & 24;
            try_move_penguin(0, -1, 0);
            try_move_penguin(1, 1, 0);
        }
        if(player1_buttons & INPUT_MASK_RIGHT) {
            penguin_gx[0] = FACE_RIGHT;
            penguin_gx[1] = FACE_LEFT + PINK_PENGUIN;
            penguin_gy[0] = global_tick & 24;
            penguin_gy[1] = global_tick & 24;
            try_move_penguin(0, 1, 0);
            try_move_penguin(1, -1, 0);
        }
        if(player1_buttons & INPUT_MASK_UP) {
            penguin_gx[0] = FACE_UP;
            penguin_gx[1] = FACE_UP + PINK_PENGUIN;
            penguin_gy[0] = global_tick & 24;
            penguin_gy[1] = global_tick & 24;
            try_move_penguin(0, 0, -1);
            try_move_penguin(1, 0, -1);
        }
        if(player1_buttons & INPUT_MASK_DOWN) {
            penguin_gx[0] = FACE_DOWN;
            penguin_gx[1] = FACE_DOWN + PINK_PENGUIN;
            penguin_gy[0] = global_tick & 24;
            penguin_gy[1] = global_tick & 24;
            try_move_penguin(0, 0, 1);
            try_move_penguin(1, 0, 1);
        }

        if(((penguin_x[0] >> 3) == (penguin_x[1] >> 3)) &&
            ((penguin_y[0] >> 3) == (penguin_y[1] >> 3)) &&
            (((penguin_x[0] - MAZE_OFFSET_X_PIX) >> 3) == 7) &&
            (((penguin_y[0]) >> 3) == MAZE_OFFSET_ROW)) {
                field[71] = 128;
                penguin_gy[0] = 40;
                penguin_gy[1] = 40;
                ++level_num;
                init_field();
                init_penguins();
            }
        
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
            ++level_num;
            init_field();
            init_penguins();
        }

        draw_field(0);
        draw_sprite_now(penguin_x[0], penguin_y[0], 8, 8, penguin_gx[0], penguin_gy[0], 0);
        wait();
        draw_sprite_now(penguin_x[1], penguin_y[1], 8, 8, penguin_gx[1], penguin_gy[1], 0);
        wait();
        PROFILER_END(0);
        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }
}