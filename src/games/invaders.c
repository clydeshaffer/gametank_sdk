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

#define SHIP_Y 108
#define BULLET_SPAWN_OFFSET 8
#define BULLET_COUNT 16
static char bullet_x[BULLET_COUNT];
static char bullet_y[BULLET_COUNT];
static char next_bullet;

#define ENEMY_COUNT 5
static char enemy_group_x;
static char enemy_x[ENEMY_COUNT];
static char enemy_y[ENEMY_COUNT];
static char enemy_type[ENEMY_COUNT];
static char bg_x, bg_y;

void draw_bullets() {
    for(i = 0; i < BULLET_COUNT; ++i) {
        if(bullet_y[i]) {
            draw_box_now(bullet_x[i], bullet_y[i], 1, 3, 61);
            bullet_y[i] -= 2;
        }
    }
}

void draw_enemies() {
    for(i = 0; i < ENEMY_COUNT; ++i) {
        if(enemy_type[i]) {
            draw_sprite_frame(&ASSET__gfx__bug_json, enemy_x[i] + enemy_group_x, enemy_y[i], (global_tick >> 2) & 15, 0, 1);
        }
    }
}

void run_invaders_game() {
    await_draw_queue();
    sleep(1);
    flip_pages();
    
    global_tick = 0;
    bg_x = 0;
    bg_y = 0;

    load_spritesheet(&ASSET__gfx__ship_bmp, 0);
    load_spritesheet(&ASSET__gfx__bug_bmp, 1);
    load_big_spritesheet(&ASSET__gfx2__space, 2);
    rnd_seed = 234;

    rotation = 16;
    ship_x.b.msb = 64;
    ship_x.b.lsb = 0;
    ship_vx = 128;

    for(i = 0; i < ENEMY_COUNT; ++i) {
        enemy_type[i] = 1;
        enemy_x[i] = (i << 4);
        enemy_y[i] = 24 + ((i & 1) * 20);
    }
    enemy_group_x = 40;

    while(1) {
        update_inputs();
        bg_y -= global_tick & 1;
        draw_sprite(0, 0, 127, 127, bg_x, bg_y, 2);
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

        if((global_tick & 3) == 3) {
            enemy_group_x += ((global_tick & 64) >> 5) - 1;
        }

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
