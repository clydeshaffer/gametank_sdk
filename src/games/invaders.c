#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx.h"
#include "../gen/assets/gfx2.h"
#include "random.h"
#include "music.h"

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
static char enemy_x[MAX_ENEMY_COUNT];
static char enemy_y[MAX_ENEMY_COUNT];
static char enemy_hp[MAX_ENEMY_COUNT];
static char enemy_count;
static char enemy_type[MAX_ENEMY_COUNT];
static char bg_x, bg_y;

static char enemy_hp_for_type[3] = { 0, 8, 1 };

void enemy_formation_1() {
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

void enemy_formation_2() {
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

void draw_bullets() {
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

void draw_enemies() {
    for(i = 0; i < MAX_ENEMY_COUNT; ++i) {
        switch(enemy_type[i]) {
            case 1:
                draw_sprite_frame(&ASSET__gfx__bug_json, enemy_x[i] + enemy_group_x, enemy_y[i], (global_tick >> 2) & 3, 0, 1);
                break;
            case 2:
                draw_sprite_frame(&ASSET__gfx__smolbug_json, enemy_x[i] + enemy_group_x, enemy_y[i], (global_tick >> 1) & 3, 0, 2);
                break;
        }
    }
}

char delta(char a, char b) {
    if(a > b) return a - b;
    return b - a;
}

void move_enemies() {
    if((global_tick & 3) == 3) {
        enemy_group_x += ((global_tick & 64) >> 5) - 1;
    }
    if(bullet_y[highest_bullet]) {
        for(i = 0; i < MAX_ENEMY_COUNT; ++i) {
            if(enemy_type[i]) {
                if((bullet_y[highest_bullet] - enemy_y[i]) < 8) {
                    if(delta(enemy_x[i] + enemy_group_x, bullet_x[highest_bullet]) < 8) {
                        --enemy_hp[i];
                        if(enemy_hp[i] == 0) {
                            enemy_type[i] = 0;
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
    if(enemy_count == 0) {
        enemy_formation_2();
    }
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

    load_spritesheet(&ASSET__gfx__ship_bmp, 0);
    load_spritesheet(&ASSET__gfx__bug_bmp, 1);
    load_spritesheet(&ASSET__gfx__smolbug_bmp, 2);
    load_big_spritesheet(&ASSET__gfx2__space, 3);
    rnd_seed = 234;

    rotation = 16;
    ship_x.b.msb = 64;
    ship_x.b.lsb = 0;
    ship_vx = 128;
    next_bullet = 0;
    highest_bullet = next_bullet - 1;

    enemy_formation_1();

    while(1) {
        update_inputs();
        bg_y -= global_tick & 1;
        draw_sprite(0, 0, 127, 127, bg_x, bg_y, 3);
        await_draw_queue();

        if(player1_buttons & INPUT_MASK_LEFT) {
            ship_vx -= 4;
        }

        if(player1_buttons & INPUT_MASK_RIGHT) {
            ship_vx += 4;
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

        ship_x.i += (ship_vx - 128) * 2;
        if(ship_x.b.msb > 118) {
            ship_x.b.msb = 118;
            ship_x.b.lsb = 0;
        }
        if(ship_x.b.msb < 10) {
            ship_x.b.msb = 10;
            ship_x.b.lsb = 0;
        }

        move_enemies();

        draw_sprite_frame(&ASSET__gfx__ship_json, ship_x.b.msb, SHIP_Y, rotation, 0, 0);
        draw_enemies();
        wait();

        draw_bullets();

        clear_border(0);
        await_draw_queue();

        sleep(1);
        flip_pages();
        ++global_tick;
        tick_music();
    }
}
