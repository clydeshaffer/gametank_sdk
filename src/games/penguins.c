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
#include "banking.h"
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

#define PENGUIN_STATUS_NORMAL 0
#define PENGUIN_STATUS_STUCK 1
#define PENGUIN_STATUS_RAY 2

#define PENGUIN_GY_RAY 32

static char penguin_x[2];
static char penguin_y[2];
static char penguin_status[2];
static char penguin_gx[2];
static char penguin_gy[2];
static char ctrl_penguin;
static char last_dx[2];
static char last_dy[2];
static int countdown;

#define MAX_SPIDERS 4
static char spider_x[MAX_SPIDERS];
static char spider_y[MAX_SPIDERS];
static char spider_state[MAX_SPIDERS];
static char spider_dirs_x[5] = { 0, 1, 0, -1, 0};
static char spider_dirs_y[5] = { 0, 0, 1, 0, -1};

static void init_penguins() {
    penguin_x[0] = MAZE_OFFSET_X_PIX + (6 * 8);
    penguin_x[1] = MAZE_OFFSET_X_PIX + (8 * 8);
    penguin_y[0] = (MAZE_OFFSET_ROW + 9) * 8;
    penguin_y[1] = penguin_y[0];
    penguin_gx[0] = FACE_DOWN;
    penguin_gx[1] = FACE_DOWN + PINK_PENGUIN;
    penguin_gy[0] = 0;
    penguin_gy[1] = 0;
    penguin_status[0] = PENGUIN_STATUS_NORMAL;
    penguin_status[1] = PENGUIN_STATUS_NORMAL;
}

static void try_scan_ray(char pid) {
    c = last_dx[pid] + ((penguin_x[pid] + 4 - MAZE_OFFSET_X_PIX) >> 3);
    r = last_dy[pid] + (((penguin_y[pid] + 4) >> 3));
    i = field[(r << 4) + c];
    if(i == TILE_WEB) {
        field[(r << 4) + c] = 0;
        do_noise_effect(30,100,3);
        score += 1;
    }
    for(i = 0; i < MAX_SPIDERS; ++i) {
        if(spider_state[i]) {
            if(c == ((spider_x[i] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                if(r == ((spider_y[i] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                    spider_state[i] = 0;
                    do_noise_effect(80,-10,6);
                    score += 5;
                }
            }
        }
    }
}

#pragma codeseg(push, "PROG0")
static void _try_move_penguin(char pid, char dx, char dy) {
    if(dx | dy) {
        last_dx[pid] = dx;
        last_dy[pid] = dy;
    }
    if(penguin_status[pid] == PENGUIN_STATUS_STUCK) {
        c = (penguin_x[pid] + 4 - MAZE_OFFSET_X_PIX) >> 3;
        r = (((penguin_y[pid] + 4) >> 3));
        i = field[(r << 4) + c];
        if(i == TILE_WEB) {
            return;
        }
        penguin_status[pid] = PENGUIN_STATUS_NORMAL;
        penguin_gy[pid] -= 40;
    }
    if(penguin_status[pid] == PENGUIN_STATUS_RAY) {
        return;
    }
    if(!(dx | dy)) {
        return;
    }
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
                        c = (penguin_x[pid] + 4 - MAZE_OFFSET_X_PIX) >> 3;
                        r = (((penguin_y[pid] + 4) >> 3));
                        i = field[(r << 4) + c];
                        if(i == TILE_WEB) {
                            penguin_status[pid] = PENGUIN_STATUS_STUCK;
                        } else if(i == TILE_HEART) {
                            field[(r << 4) + c] = 0;
                            score += 10;
                        }
                        return;
                    }
                }
            }
        }
    }
    penguin_x[pid] -= dx;
    penguin_y[pid] -= dy;
}

static void _move_spiders() {
    char wall_count;
    i = global_tick % MAX_SPIDERS;
    if(spider_state[i]) {
        
        if(!(((spider_x[i] - MAZE_OFFSET_X_PIX) | spider_y[i]) & 7)) {
            c = (spider_x[i] + 4 - MAZE_OFFSET_X_PIX) >> 3;
            r = (((spider_y[i] + 4) >> 3));
            wall_count = (field[((r + 1) << 4) + c + 0] & 8) +
            (field[((r + 0) << 4) + c + 1] & 8) +
            (field[((r - 1) << 4) + c + 0] & 8) +
            (field[((r + 0) << 4) + c - 1] & 8);

            if(c == 0) wall_count += 8;
            if(c == 14) wall_count += 8;
            if(r == 4) wall_count += 8;
            if(r == 13) wall_count += 8;
            if(wall_count < 16) spider_state[i] = (rnd()&3)+1;
        }

        spider_x[i] += spider_dirs_x[spider_state[i]];
        spider_y[i] += spider_dirs_y[spider_state[i]];

        if((spider_x[i] != 3) &&
            (spider_x[i] != 117) &&
            (spider_y[i] != 31) &&
            (spider_y[i] != 105)) {
            c = (spider_x[i] - MAZE_OFFSET_X_PIX) >> 3;
            r = (((spider_y[i]) >> 3));
            if(!(field[(r << 4) + c] & 8)) {
                c = (spider_x[i] + 7 - MAZE_OFFSET_X_PIX) >> 3;
                if(!(field[(r << 4) + c] & 8)) {
                    r = (((spider_y[i] + 7) >> 3));
                    if(!(field[(r << 4) + c] & 8)) {
                        c = (spider_x[i] - MAZE_OFFSET_X_PIX) >> 3;
                        if(!(field[(r << 4) + c] & 8)) {                   

                            c = (spider_x[i] + 4 - MAZE_OFFSET_X_PIX) >> 3;
                            r = (((spider_y[i] + 4) >> 3));

                            if(c == ((penguin_x[0] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                                if(r == ((penguin_y[0] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                                    penguin_status[0] = PENGUIN_STATUS_STUCK;
                                    penguin_gx[0] = 8;
                                    penguin_gy[0] = 40 + (global_tick & 8);
                                    if(game_state != GAME_STATE_FAIL) {
                                        game_state = GAME_STATE_FAIL;
                                        stop_music();
                                        play_song(&ASSET__music__peng_oops_mid, REPEAT_NONE);
                                    }
                                }
                            } else if(c == ((penguin_x[1] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                                if(r == ((penguin_y[1] + 4 - MAZE_OFFSET_X_PIX) >> 3)) {
                                    penguin_status[1] = PENGUIN_STATUS_STUCK;
                                    penguin_gx[1] = 8 + PINK_PENGUIN;
                                    penguin_gy[1] = 40 + (global_tick & 8);
                                    if(game_state != GAME_STATE_FAIL) {
                                        game_state = GAME_STATE_FAIL;
                                        stop_music();
                                        play_song(&ASSET__music__peng_oops_mid, REPEAT_NONE);
                                    }
                                }
                            }

                            return;
                        }
                    }
                }
            }
        }
        spider_x[i] -= spider_dirs_x[spider_state[i]];
        spider_y[i] -= spider_dirs_y[spider_state[i]];
        spider_state[i] = (rnd()&3)+1;
    }
        
}

static const char top_row[16] = {
        224, 225, 226, 240, 240, 240, 240, 227, 228, 0, 0, 0, 229, 0, 0, 0
};

static void _print_num_tiles(char c, int num) {
    while(num > 0) {
        field[32 + c] = 240 + (num % 10);
        num /= 10;
        --c;
    }
}

static void _update_field_texts() {
    for(i = 0; i < 16; ++i) {
        field[i+32] = top_row[i];
    }
    _print_num_tiles(6, score);
    _print_num_tiles(11, countdown/10);
    _print_num_tiles(14, level_num);
}

#pragma codeseg(pop)

static void try_move_penguin(char pid, char dx, char dy) {
    change_rom_bank(0xFE);
    _try_move_penguin(pid, dx, dy);
}

static void move_spiders() {
    change_rom_bank(0xFE);
    _move_spiders();
}

static void update_field_texts() {
    change_rom_bank(0xFE);
    _update_field_texts();
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
    load_level_num(ctrl_penguin);
    if(is_bonus_stage()) {
        for(i = 0; i < MAX_SPIDERS; ++i) {
            spider_state[i] = 0;
        }    
    } else {
        for(i = 0; i < MAX_SPIDERS; ++i) {
            if((1 << i) & level_num) {
                spider_state[i] = 1;
                spider_y[i] = 32;
                spider_x[i] = 4 + (i << 5);
            } else {
                spider_state[i] = 0;
            }
        }
    }
    countdown = 3600;
    update_field_texts();
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
    load_spritesheet(&ASSET__gfx6__peng_end_bmp, 2);

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
            wait();
            draw_sprite_now(48, 72 + (ctrl_penguin << 4), 8, 8, FACE_DOWN + (ctrl_penguin << 5), 0, 0);
             if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                level_num = 1;
                init_field();
                init_penguins();
                game_state = GAME_STATE_LEVEL_INTRO;
                global_tick = 0;
                lives = 3;
            }
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_DOWN) {
                ctrl_penguin = 1;
            }
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_UP) {
                ctrl_penguin = 0;
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
        } else if(game_state == GAME_STATE_GAMEOVER) {
            clear_border(0);
            await_draw_queue();
            draw_sprite_now(0, 0, 127, 127, 0, 0, 2);
        } else {
            //Top
            draw_tile(4, 24, 120, 8, 96, 0, 0);
            //Bottom
            draw_tile(4, SCREEN_HEIGHT-16, 120, 16, 96, 0, 0);
            //Left
            draw_tile(0, 24, 4, 104, 96, 0, 0);
            //Right
            draw_tile(SCREEN_WIDTH-4, 24, 4, 104, 96, 0, 0);

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

                if(player1_buttons & INPUT_MASK_A) {
                    if(penguin_status[0] != PENGUIN_STATUS_STUCK) {
                        try_scan_ray(0);
                        penguin_status[0] = PENGUIN_STATUS_RAY;
                        penguin_gy[0] = PENGUIN_GY_RAY;
                    }
                    if(penguin_status[1] != PENGUIN_STATUS_STUCK) {
                        try_scan_ray(1);
                        penguin_status[1] = PENGUIN_STATUS_RAY;
                        penguin_gy[1] = PENGUIN_GY_RAY;
                    }
                } else {
                    if(penguin_status[0] == PENGUIN_STATUS_RAY) {
                        penguin_status[0] = PENGUIN_STATUS_NORMAL;
                        penguin_gy[0] -= PENGUIN_GY_RAY;
                    }
                    if(penguin_status[1] == PENGUIN_STATUS_RAY) {
                        penguin_status[1] = PENGUIN_STATUS_NORMAL;
                        penguin_gy[1] -= PENGUIN_GY_RAY;
                    }
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

            if(penguin_status[0] == PENGUIN_STATUS_STUCK) {
                penguin_gx[0] = 8;
                penguin_gy[0] = 40 + (global_tick & 8);
                last_dx[0] = 0;
                last_dy[0] = 1;
                if(game_state != GAME_STATE_FAIL) {
                    try_move_penguin(0, 0, 0);
                }
            }

            if(penguin_status[1] == PENGUIN_STATUS_STUCK) {
                penguin_gx[1] = 8 + PINK_PENGUIN;
                penguin_gy[1] = 40 + (global_tick & 8);
                last_dx[1] = 0;
                last_dy[1] = 1;
                if(game_state != GAME_STATE_FAIL) {
                    try_move_penguin(1, 0, 0);
                    if(penguin_status[0] == penguin_status[1]) {
                        game_state = GAME_STATE_FAIL;
                        stop_music();
                        play_song(&ASSET__music__peng_oops_mid, REPEAT_NONE);
                    }
                }
            }

            if(game_state == GAME_STATE_PLAYING) {
                move_spiders();
                --countdown;
                if(!countdown) {
                    penguin_gx[0] = 8;
                    penguin_gy[0] = 40 + (global_tick & 8);
                    penguin_gx[1] = 8 + PINK_PENGUIN;
                    penguin_gy[1] = 40 + (global_tick & 8);
                    game_state = GAME_STATE_FAIL;
                    stop_music();
                    play_song(&ASSET__music__peng_oops_mid, REPEAT_NONE);
                }
            }

            update_field_texts();

            await_draw_queue();

            draw_field(0);
            draw_sprite_now(penguin_x[0], penguin_y[0], 8, 8, penguin_gx[0], penguin_gy[0], 0);
            wait();
            draw_sprite_now(penguin_x[1], penguin_y[1], 8, 8, penguin_gx[1], penguin_gy[1], 0);
            wait();
            if(penguin_status[0] == PENGUIN_STATUS_RAY) {
                draw_sprite_now(penguin_x[0] + (last_dx[0] << 3), penguin_y[0] + (last_dy[0] << 3), 8, 8, penguin_gx[0], 56, 0);
                wait();
            }
            if(penguin_status[1] == PENGUIN_STATUS_RAY) {
                draw_sprite_now(penguin_x[1] + (last_dx[1] << 3), penguin_y[1] + (last_dy[1] << 3), 8, 8, penguin_gx[1], 56, 0);
                wait();
            }

            for(i = 0; i < MAX_SPIDERS; ++i) {
                if(spider_state[i]) {
                    draw_sprite_now(spider_x[i] & 127, spider_y[i] & 127, 8, 8, global_tick & 8, 80, 0);
                    wait();
                }
            }
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
            } else if(game_state == GAME_STATE_FAIL) {
                if(is_bonus_stage()) {
                    ++level_num;
                } else {
                    --lives;
                }
                if(lives) {
                    init_field();
                    init_penguins();
                    game_state = GAME_STATE_LEVEL_INTRO;
                    global_tick = 0;
                } else {
                    game_state = GAME_STATE_GAMEOVER;
                    play_song(&ASSET__music__peng_gameover_mid, REPEAT_NONE);
                }
            } else if(game_state == GAME_STATE_GAMEOVER) {
                game_state = GAME_STATE_TITLE;
            }
        }
    }
}