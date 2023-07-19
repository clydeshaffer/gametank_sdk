#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx.h"
#include "../gen/assets/gfx2.h"
#include "random.h"
#include "music.h"
#include "banking.h"

static char rotation;

static coordinate ship_x;
static unsigned char ship_vx;
static char i;
static char highest_bullet;

#define SHIP_Y 108
#define BULLET_SPAWN_OFFSET 8
#define BULLET_COUNT 16
static char bullet_x[BULLET_COUNT];
static char bullet_y[BULLET_COUNT];
static char next_bullet;

#define MAX_ENEMY_COUNT 12
static char enemy_group_x;
static char enemy_group_y;
static char enemy_x[MAX_ENEMY_COUNT];
static char enemy_y[MAX_ENEMY_COUNT];
static char enemy_hp[MAX_ENEMY_COUNT];
static char enemy_count;
static char enemy_type[MAX_ENEMY_COUNT];
static char bg_x, bg_y;

#define ENEMY_EXPLOSION_FRAMES 9

static char enemy_hp_for_type[3] = { 0, 8, 1 };
static char enemy_hitbox_size[3] = { 0, 16, 8};
static char attacking_enemy;
static char attacking_enemy_vy;
static char attacking_enemy_start_y;
static char attack_tick_mask;

#define INVADERS_STATE_TITLE 0
#define INVADERS_STATE_PLAYING 1
#define INVADERS_STATE_DEATH 2
#define INVADERS_STATE_GAMEOVER 3

static void enemy_formation_1() {
    global_tick = 0;
    enemy_count = 12;
    for(i = 0; i < 12; ++i) {
        enemy_type[i] = 2;
        enemy_hp[i] = enemy_hp_for_type[enemy_type[i]];
        enemy_x[i] = ((i % 6) << 4);
        enemy_y[i] = (i > 5) ? 44 : 24;
    }
    enemy_group_x = 32;
}

static void enemy_formation_2() {
    global_tick = 0;
    enemy_count = 7;
    enemy_type[0] = 1;
    enemy_hp[0] = enemy_hp_for_type[1];
    enemy_x[0] = 40;
    enemy_y[0] = 24;
    for(i = 1; i < 7; ++i) {
        enemy_type[i] = 2;
        enemy_hp[i] = enemy_hp_for_type[enemy_type[i]];
        enemy_x[i] = ((i % 6) << 4);
        enemy_y[i] = 44;
    }
    enemy_group_x = 32;
}

static void enemy_formation_3() {
    global_tick = 0;
    enemy_count = 5;
    for(i = 0; i < 5; ++i) {
        enemy_type[i] = 1 + (i & 1);
        enemy_hp[i] = enemy_hp_for_type[enemy_type[i]];
        enemy_x[i] = (i << 4);
        enemy_y[i] = 24 + ((i & 1) * 20);
    }
    enemy_group_x = 40;
}

static void enemy_formation_4() {
    global_tick = 0;
    enemy_count = 5;
    for(i = 0; i < 5; ++i) {
        enemy_type[i] = 1;
        enemy_hp[i] = enemy_hp_for_type[enemy_type[i]];
        enemy_x[i] = (i << 4);
        enemy_y[i] = 24 + ((i & 1) * 20);
    }
    enemy_group_x = 40;
}

static void enemy_formation_n(char n) {
    switch(n & 3) {
        case 0:
            enemy_formation_1();
            break;
        case 1:
            enemy_formation_2();
            break;
        case 2:
            enemy_formation_3();
            break;
        case 3:
            enemy_formation_4();
            break;
    }
    enemy_group_y = 0 - 64;
    attacking_enemy = -1;
    attacking_enemy_vy = 0;
    if(n > 32) attack_tick_mask = 255;
    else attack_tick_mask = 128 >> (n >> 2);
    
}

static void clear_enemies() {
    for(i = 0; i < MAX_ENEMY_COUNT; ++i) {
        enemy_type[i] = 0;
    }
    enemy_count = 0;
}

static void draw_bullets() {
    highest_bullet = 0;
    for(i = 0; i < BULLET_COUNT; ++i) {
        if(bullet_y[i]) {
            draw_box_now(bullet_x[i], bullet_y[i], 1, 3, 61);
            bullet_y[i] -= 2;
            if(bullet_y[i]) {
                if(bullet_y[highest_bullet] == 0) highest_bullet = i;
                else if(bullet_y[i] < bullet_y[highest_bullet])
                    highest_bullet = i;
            }
        }
    }
}

static void draw_enemies() {
    for(i = 0; i < MAX_ENEMY_COUNT; ++i) {
        switch(enemy_type[i]) {
            case 0:
                if(enemy_hp[i]) {
                    draw_sprite_frame(&ASSET__gfx__explosion_f_json, enemy_x[i] + enemy_group_x, enemy_y[i] + enemy_group_y, ENEMY_EXPLOSION_FRAMES - enemy_hp[i], 0, 5 | BANK_CLIP_Y | BANK_CLIP_X);
                    enemy_hp[i] -= (global_tick & 1);
                }
                break;
            case 1:
                draw_sprite_frame(&ASSET__gfx__bug_json, enemy_x[i] + enemy_group_x, enemy_y[i] + enemy_group_y, (global_tick >> 2) & 3, 0, 1 | BANK_CLIP_Y);
                break;
            case 2:
                draw_sprite_frame(&ASSET__gfx__smolbug_json, enemy_x[i] + enemy_group_x, enemy_y[i] + enemy_group_y, (global_tick >> 1) & 3, 0, 2 | BANK_CLIP_Y);
                break;
        }
    }
}

#pragma codeseg(push, "PROG0")

static void move_enemies() {
    if(enemy_group_y != 0) ++enemy_group_y;
    if((global_tick & 3) == 3) {
        enemy_group_x += ((global_tick & 64) >> 5) - 1;
    }
    if(bullet_y[highest_bullet]) {
        for(i = 0; i < MAX_ENEMY_COUNT; ++i) {
            if(enemy_type[i]) {
                if((bullet_y[highest_bullet] + enemy_group_y - enemy_y[i]) < 8) {
                    if(delta(enemy_x[i] + enemy_group_x, bullet_x[highest_bullet]) < 8) {
                        --enemy_hp[i];
                        if(enemy_hp[i] == 0) {
                            enemy_type[i] = 0;
                            enemy_hp[i] = ENEMY_EXPLOSION_FRAMES;
                            if(attacking_enemy == i) {
                                attacking_enemy = -1;
                            }
                            do_noise_effect(80, -30, 10);
                            --enemy_count;
                        }
                        bullet_y[highest_bullet] = 0;
                        highest_bullet = (highest_bullet + 1) % BULLET_COUNT;
                        break;
                    }
                }
            }
        }
    }
    if(game_state == INVADERS_STATE_PLAYING) {
        if(enemy_count == 0) {
            ++level_num;
            enemy_formation_n(level_num);
        }
    }

    if(attacking_enemy != 255) {
        enemy_y[attacking_enemy] += attacking_enemy_vy;
        if(enemy_y[attacking_enemy] < attacking_enemy_start_y) {
            enemy_y[attacking_enemy] = attacking_enemy_start_y;
            attacking_enemy = 255;
        } else if(enemy_y[attacking_enemy] > SHIP_Y) {
            attacking_enemy_vy = 255;
            if(delta(ship_x.b.msb, enemy_x[attacking_enemy] + enemy_group_x) < enemy_hitbox_size[enemy_type[attacking_enemy]]) {
                game_state = INVADERS_STATE_DEATH;
                rotation = 0;
                do_noise_effect(30, 32, 40);
            }
        }
    } else if((global_tick & attack_tick_mask) && (enemy_count > 0) && (game_state == INVADERS_STATE_PLAYING)) { 
        attacking_enemy = rnd() % MAX_ENEMY_COUNT;
        attacking_enemy_vy = 2;
        while(enemy_type[attacking_enemy] == 0) {
            attacking_enemy = (attacking_enemy+1) % MAX_ENEMY_COUNT;
        }
        attacking_enemy_start_y = enemy_y[attacking_enemy];
    }
    
}

#pragma codeseg(pop)

static void init_player() {
    rotation = 16;
    ship_x.b.msb = 64;
    ship_x.b.lsb = 0;
    ship_vx = 128;
    next_bullet = 0;
    highest_bullet = next_bullet - 1;
}

void run_invaders_game() {
    await_draw_queue();

    text_cursor_x = 16;
    text_print_width = 128;
    text_cursor_y = 108;
    text_use_alt_color = 1;
    print_text("Loading...");
    flip_pages();
    text_cursor_x = 16;
    text_print_width = 128;
    text_cursor_y = 108;
    text_use_alt_color = 1;
    print_text("Loading...");
    flip_pages();
    sleep(1);

    global_tick = 0;
    bg_x = 0;
    bg_y = 0;
    game_state = INVADERS_STATE_TITLE;

    load_spritesheet(&ASSET__gfx__ship_bmp, 0);
    load_spritesheet(&ASSET__gfx__bug_bmp, 1);
    load_spritesheet(&ASSET__gfx__smolbug_bmp, 2);
    load_big_spritesheet(&ASSET__gfx2__space, 3);
    load_wide_spritesheet(&ASSET__gfx__explosion_d, 4);
    load_spritesheet(&ASSET__gfx__explosion_f_bmp, 5);
    load_spritesheet(&ASSET__gfx__invaders_title_bmp, 6);
    rnd_seed = 234;

    init_player();

    clear_enemies();

    while(1) {
        update_inputs();
        bg_y -= global_tick & 1;
        draw_sprite(0, 0, 127, 127, bg_x, bg_y, 3);
        await_draw_queue();

        if(game_state == INVADERS_STATE_TITLE) {
            draw_sprite(0, 48, 127, 26, 0, 0, 6);
            draw_sprite(37, 96, 53, 7, 0, 32, 6);
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                level_num = 0;
                enemy_formation_n(level_num);
                game_state = INVADERS_STATE_PLAYING;
                lives = 3;
                global_tick = 0;
                init_player();
            }
        } else if(game_state == INVADERS_STATE_PLAYING) {
            if(player1_buttons & INPUT_MASK_LEFT) {
                ship_vx -= 8;
            }

            if(player1_buttons & INPUT_MASK_RIGHT) {
                ship_vx += 8;
            }

            if(!(player1_buttons & (INPUT_MASK_LEFT | INPUT_MASK_RIGHT))) {
                if(ship_vx > 128) ship_vx--;
                if(ship_vx < 128) ship_vx++;
            }

            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A)  {
                bullet_x[next_bullet] = ship_x.b.msb;
                bullet_y[next_bullet] = SHIP_Y-BULLET_SPAWN_OFFSET;
                highest_bullet = (highest_bullet + 1) % BULLET_COUNT;
                next_bullet = (next_bullet + 1) % BULLET_COUNT;
                do_noise_effect(90, 150, 3);
            }

            if(ship_vx > 192) ship_vx = 192;
            if(ship_vx < 64) ship_vx = 64;
            rotation = (ship_vx - 64) >> 2;

            ship_x.i += (ship_vx - 128) * 4;
            if(ship_x.b.msb > 118) {
                ship_x.b.msb = 118;
                ship_x.b.lsb = 0;
            }
            if(ship_x.b.msb < 10) {
                ship_x.b.msb = 10;
                ship_x.b.lsb = 0;
            }
            draw_sprite_frame(&ASSET__gfx__ship_json, ship_x.b.msb, SHIP_Y, rotation, 0, 0);
        } else if(game_state == INVADERS_STATE_DEATH) {
            if(rotation < 10) {
                draw_sprite_frame(&ASSET__gfx__explosion_d_json, ship_x.b.msb, SHIP_Y, rotation, 0, 4 | BANK_CLIP_X | BANK_CLIP_Y);
            } else if(rotation > 45) {
                init_player();
                game_state = INVADERS_STATE_PLAYING;
                --lives;
                if(lives == 0) {
                    game_state = INVADERS_STATE_GAMEOVER;
                    rotation = 0;
                }
            }
            rotation += (global_tick & 3) == 3;
        } else if(game_state == INVADERS_STATE_GAMEOVER) {
            draw_sprite(27, 58, 74, 13, 0, 48, 6);
            ++rotation;
            if(enemy_group_y != -64) {
                enemy_group_y -= 2;
            }
            if(rotation == 64) {
                clear_enemies();
            }
            if(!rotation) {
                game_state = INVADERS_STATE_TITLE;
            }
        }

        change_rom_bank(0xFE);
        move_enemies();

        draw_enemies();
        await_draw_queue();

        draw_bullets();

        clear_border(0);
        await_draw_queue();

        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }
}
