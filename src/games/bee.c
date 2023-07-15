#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx5.h"
#include "../gen/assets/music.h"
#include "random.h"
#include "music.h"

#define HIVE_X 32
#define HIVE_Y 96

static char bee_x;
static char bee_y;
static char bee_flip;
static char bee_alive;
static char played_eat_sound;
static char nectar_load;
static char bird_x;
static char bird_y;
static char bird_vx;
static char bird_vy;
static char bird_flip;
static char bird_timer;

#define STATE_TITLE 0
#define STATE_PLAY 1
#define STATE_GAMEOVER 2

#define FLOWER_FRAME 2
#define BIRD_FRAME 3
#define HIVE_FRAME 5

#define EAT_RANGE 4
#define BIRD_RANGE 12
#define FLOWER_COUNT 4
static char flower_x[FLOWER_COUNT];
static char flower_y[FLOWER_COUNT];
static char flower_nectar[FLOWER_COUNT];
static char i,k;

#define LIMIT_LEFT 8
#define LIMIT_RIGHT 120
#define LIMIT_TOP 32
#define LIMIT_BOTTOM 112

static void place_flower(char i) {
    flower_x[i] = rnd_range(LIMIT_LEFT, LIMIT_RIGHT);
    flower_y[i] = rnd_range(85, LIMIT_BOTTOM - 16);
    flower_nectar[i] = 255;
}

static void place_flowers() {
    for(i = 0; i < FLOWER_COUNT; ++i) {
        rnd();
        rnd();
        rnd();
        place_flower(i);
    }
}

static void draw_flowers() {
    for(i = 0; i < FLOWER_COUNT; ++i) {
        draw_sprite_frame(&ASSET__gfx5__bees_json, flower_x[i], flower_y[i], FLOWER_FRAME, 0, BANK_CLIP_X | BANK_CLIP_Y);
    }
}

static char bee_near_flower() {
    for(i = 0; i < FLOWER_COUNT; ++i) {
        if(delta(bee_x, flower_x[i]) < EAT_RANGE) {
            if(delta(bee_y, flower_y[i]) < EAT_RANGE) {
                return i;
            }
        }
    }
    return -1;
}

static char bee_near_hive() {
    return (delta(bee_x, HIVE_X) < EAT_RANGE) && (delta(bee_y, HIVE_Y) < EAT_RANGE);
}

static char bee_near_bird() {
    return (delta(bee_x, bird_x) < BIRD_RANGE) && (delta(bee_y, bird_y) < BIRD_RANGE);
}

static char score_text[32];

static void init_texts() {
    score_text[0] = '0';
    score_text[1] = 0;
}

static void update_texts() {
    if(score) {
        i = score;
        k = 1;
        while(i > 0) {
            score_text[32-k] = '0' + (i % 10);
            i /= 10;
            ++k;
        }
        for(i = 0; i < (k-1); ++i) {
            score_text[i] = score_text[31-i];
        }
        score_text[k] = 0;
    } else {
        score_text[7] = '0';
        score_text[8] = 0;
    }
}

static void init_round() {
    bird_timer = 0;
    place_flowers();
    bird_vx = 1;
    bird_vy = 1;
    bird_x = 64;
    bird_y = 32;

    bee_x = HIVE_X;
    bee_y = HIVE_Y;
    bee_flip = 0;
    nectar_load = 0;
    played_eat_sound = 0;
    bee_alive = 1;
    score = 0;
    init_texts();
}

void run_bee_game() {

    await_draw_queue();
    flip_pages();
    sleep(1);
    load_spritesheet(&ASSET__gfx5__bees_bmp, 0);
    load_spritesheet(&ASSET__gfx5__meadow_bmp, 1);
    load_spritesheet(&ASSET__gfx5__bee_titles_bmp, 2);
    load_spritesheet(&ASSET__gfx5__bees_font_bmp, 7);
    rnd_seed = 234;

    game_state = STATE_TITLE;
    init_round();

    while(1) {
        update_inputs();
        draw_sprite(0, 0, 127, 127, 0, 0, 1);

        if(game_state == STATE_TITLE) {
            draw_sprite(0, 32, 127, 24, 0, 0, 2);
            draw_sprite(0, 72, 127, 9, 0, 32, 2);
            clear_border(0);
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                game_state = STATE_PLAY;
                init_round();
            }
        } else if(game_state == STATE_PLAY) {
            draw_flowers();
            draw_sprite_frame(&ASSET__gfx5__bees_json, HIVE_X, HIVE_Y, HIVE_FRAME, 0, 0);

            if(bee_alive)
                draw_sprite_frame(&ASSET__gfx5__bees_json, bee_x, bee_y, ((global_tick&2) >> 1), bee_flip, BANK_CLIP_X | BANK_CLIP_Y);
            draw_sprite_frame(&ASSET__gfx5__bees_json, bird_x, bird_y, BIRD_FRAME + ((global_tick&4) >> 2), bird_flip, BANK_CLIP_X | BANK_CLIP_Y);
            draw_sprite_frame(&ASSET__gfx5__bees_json, 32, 16, 6, 0, 0);
            draw_sprite_frame(&ASSET__gfx5__bees_json, 32, 25, 7, 0, 0);
            draw_box(44, 12, nectar_load >> 2, 8, (nectar_load == 255) ? 60 : 93);
            clear_border(0);

            if(bee_alive) {
                if(player1_buttons & INPUT_MASK_LEFT) {
                    --bee_x;
                    bee_flip = SPRITE_FLIP_X;
                }
                if(player1_buttons & INPUT_MASK_RIGHT) {
                    ++bee_x;
                    bee_flip = 0;
                }
                if(player1_buttons & INPUT_MASK_UP) {
                    --bee_y;
                }
                if(player1_buttons & INPUT_MASK_DOWN) {
                    ++bee_y;
                }

                if(bee_x < LIMIT_LEFT) bee_x = LIMIT_LEFT;
                if(bee_x > LIMIT_RIGHT) bee_x = LIMIT_RIGHT;
                if(bee_y < LIMIT_TOP) bee_y = LIMIT_TOP;
                if(bee_y > LIMIT_BOTTOM) bee_y = LIMIT_BOTTOM;

                if(nectar_load != 255) {
                    k = bee_near_flower();
                    if(k != 255) {
                        if(!played_eat_sound) {
                            played_eat_sound = 1;
                            do_noise_effect(30, 100, 10);
                        }
                        --flower_nectar[k];
                        ++nectar_load;
                        if(nectar_load == 255) {
                            do_tone_effect(3, 36, 0, 16);
                        }
                        if(flower_nectar[k] == 0) {
                            place_flower(k);
                        }
                    } else {
                        played_eat_sound = 0;
                    }
                } else {
                    if(bee_near_hive()) {
                        nectar_load = 0;
                        ++score;
                        update_texts();
                        do_tone_effect(3, 60, 10, 32);
                    }
                }

                if(bee_near_bird()) {
                    do_noise_effect(100, -32, 16);
                    bee_alive = 0;
                    state_timer = 60;
                }
            } else {
                if(state_timer) {
                    --state_timer;
                } else {
                    game_state = STATE_GAMEOVER;
                    state_timer = 255;
                }
            }

            bird_x += bird_vx;
            bird_y += bird_vy;

            if(bird_x < LIMIT_LEFT) {
                bird_vx = 1;
                bird_flip = 0;
            }
            if(bird_x > LIMIT_RIGHT) {
                bird_vx = -1;
                bird_flip = SPRITE_FLIP_X;
            }
            if(bird_y < LIMIT_TOP) {
                ++bird_y;
                bird_vy = 0;
                bird_timer = 0;
            }
            if(bird_y > LIMIT_BOTTOM) bird_vy = -1;

            if((bird_y == LIMIT_TOP)) {
                ++bird_timer;
                if(!bird_timer) {
                    bird_vy = 1;
                    do_tone_effect(1, 100, -20, 32);
                }
            }

            await_draw_queue();

            text_use_alt_color = 1;
            text_cursor_x = 44;
            text_cursor_y = 22;
            text_print_width = 128;
            print_text(score_text);
        } else if(game_state == STATE_GAMEOVER) {
            draw_sprite(0, 64, 127, 16, 0, 48, 2);
            clear_border(0);
            --state_timer;
            if(!state_timer) {
                game_state = STATE_TITLE;
            }
        }

        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }

}