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
#include "dynawave.h"

static char i;


#define PENGUINS_GY 8
#define FACE_UP 0
#define FACE_DOWN 8
#define FACE_RIGHT 16
#define FACE_LEFT 24
#define PINK_PENGUIN 32

#define GAME_STATE_TITLE 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_LEVEL_INTRO 2
#define GAME_STATE_LEVEL_START 3
#define GAME_STATE_LEVEL_END 4
#define GAME_STATE_FAIL 5
#define GAME_STATE_GAMEOVER 6

static char r,c;

static char penguin_x[2];
static char penguin_y[2];
static char penguin_status[2];
static char penguin_gx[2];
static char penguin_gy[2];
static char ctrl_penguin;

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

static char level_num_text[12];
static char lives_text[8];

void run_penguins_game() {
    init_dynawave_with_fw(0);
    await_draw_queue();
    flip_pages();
    sleep(1);

    load_spritesheet(&ASSET__gfx6__penguins_bmp, 0);
    load_spritesheet(&ASSET__gfx6__peng_title_bmp, 1);

    rnd_seed = 234;
    global_tick = 0;
    level_num = 1;
    ctrl_penguin = 0;
    init_field();
    init_penguins();

    lives_text[0] = '=';
    lives_text[1] = ' ';
    lives_text[2] = 0;

    level_num_text[0] = 'R';
    level_num_text[1] = 'O';
    level_num_text[2] = 'U';
    level_num_text[3] = 'N';
    level_num_text[4] = 'D';
    level_num_text[5] = ' ';
    level_num_text[6] = 0;

    game_state = GAME_STATE_TITLE;

    while(1) {
        PROFILER_START(0);
        update_inputs();
        draw_box(0, 0, 127, 127, 32);

        if(game_state == GAME_STATE_TITLE) {
            clear_border(0);
            await_draw_queue();
            draw_sprite_now(0, 0, 127, 127, 0, 0, 1);
             if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                level_num = 1;
                init_field();
                init_penguins();
                game_state = GAME_STATE_LEVEL_INTRO;
                global_tick = 0;
                lives = 3;
            }
        } else if(game_state == GAME_STATE_LEVEL_INTRO) {
            clear_border(0);    
            num_to_str(level_num, level_num_text+6, 6);
            num_to_str(lives, lives_text+2, 6);
            await_draw_queue();
            text_cursor_x = 32;
            text_cursor_y = 48;
            text_use_alt_color = 1;
            text_print_width = 128;
            print_text(level_num_text);

            text_cursor_x = 64;
            text_cursor_y = 64;
            print_text(lives_text);

            draw_sprite_now(48, 64, 8, 8, FACE_DOWN + (ctrl_penguin << 5), 0, 0);

            if(global_tick == 180) {
                game_state = GAME_STATE_LEVEL_START;
                play_song(&ASSET__music__peng_intro_mid, REPEAT_NONE);
            }
        } else {
            //Top
            draw_tile(4, 24, 120, 8, 96, 0, 0);
            //Bottom
            draw_tile(4, SCREEN_HEIGHT-16, 120, 16, 96, 0, 0);
            //Left
            draw_tile(0, 24, 4, 104, 96, 0, 0);
            //Right
            draw_tile(SCREEN_WIDTH-4, 24, 4, 104, 96, 0, 0);

            await_draw_queue();

            if(game_state == GAME_STATE_PLAYING) {
                if(player1_buttons & INPUT_MASK_LEFT) {
                    penguin_gx[ctrl_penguin] = FACE_LEFT;
                    penguin_gx[1 - ctrl_penguin] = FACE_RIGHT;
                    penguin_gx[1] += PINK_PENGUIN;
                    penguin_gy[ctrl_penguin] = global_tick & 24;
                    penguin_gy[1 - ctrl_penguin] = global_tick & 24;
                    try_move_penguin(ctrl_penguin, -1, 0);
                    try_move_penguin(1 - ctrl_penguin, 1, 0);
                }
                if(player1_buttons & INPUT_MASK_RIGHT) {
                    penguin_gx[ctrl_penguin] = FACE_RIGHT;
                    penguin_gx[1 - ctrl_penguin] = FACE_LEFT;
                    penguin_gx[1] += PINK_PENGUIN;
                    penguin_gy[ctrl_penguin] = global_tick & 24;
                    penguin_gy[1 - ctrl_penguin] = global_tick & 24;
                    try_move_penguin(ctrl_penguin, 1, 0);
                    try_move_penguin(1 - ctrl_penguin, -1, 0);
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
                        game_state = GAME_STATE_LEVEL_END;
                        stop_music();
                        play_song(&ASSET__music__odetojoy_mid, REPEAT_NONE);
                    }
                
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    ++level_num;
                    init_field();
                    init_penguins();
                }
            }

            draw_field(0);
            draw_sprite_now(penguin_x[0], penguin_y[0], 8, 8, penguin_gx[0], penguin_gy[0], 0);
            wait();
            draw_sprite_now(penguin_x[1], penguin_y[1], 8, 8, penguin_gx[1], penguin_gy[1], 0);
            wait();
        }
        PROFILER_END(0);
        sleep(1);
        flip_pages();
        ++global_tick;
        if(tick_music() == SONG_STATUS_ENDED) {
            if(game_state == GAME_STATE_LEVEL_END) {
                ++level_num;
                init_field();
                init_penguins();
                game_state = GAME_STATE_LEVEL_INTRO;
                global_tick = 0;
            } else if(game_state == GAME_STATE_LEVEL_START) {
                game_state = GAME_STATE_PLAYING;
                play_song(&ASSET__music__peng_loop_mid, REPEAT_LOOP);
            }
        }
    }
}