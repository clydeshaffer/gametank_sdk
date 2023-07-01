#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "gen/assets/gfx.h"
#include "gen/assets/maps.h"
#include <zlib.h>
#include "banking.h"
#include "gen/assets/mid.h"
#include "dynawave.h"
#include "music.h"

#define DEFAULT_OFFSET 4
#define ANIM_TIME 8

char tick = 0;

char anim_timer;
char anim_frame;
char anim_flip;
char anim_dir;
char player_select;
char player_x, player_y;
char offset_x, offset_y;
char move_x, move_y;
char pushing_box;

#define BARREL_TILE 252
#define BARREL_GOAL_TILE 253
#define GOAL_TILE 16
#define PLAYER_START 32
#define PLAYER_GOAL_START 24
char field[256];
char field_original[256];
char c,r,i;
char* next_level;
char* current_level;
char goals_remaining;

void scan_level() {
    i = 0;
    goals_remaining = 0;
    do {
        field_original[i] = field[i];
        if((field[i] == PLAYER_START) || (field[i] == PLAYER_GOAL_START)) {
            player_x = i & 15;
            player_y = i >> 4;
        } else if(field[i] == GOAL_TILE) {
            ++goals_remaining;
        }
        ++i;
    } while(i);
}


void init_player() {
    tick = 0;
    offset_x = DEFAULT_OFFSET;
    offset_y = DEFAULT_OFFSET;
    move_x = 0;
    move_y = 0;
    anim_timer = 0;
    anim_flip = 0;
    anim_dir = 0;
    pushing_box = 0;
}

void load_next_level() {
    change_rom_bank(ASSET__maps__microban_slc_bank);
    inflatemem(field, next_level+1);
    current_level = next_level;
    next_level += *next_level;
    next_level += 1;
    init_player();
    scan_level();
}

void draw_field() {
    i = 16;
    *dma_flags = (flagsMirror | DMA_GCARRY) & ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE);
    banksMirror = bankflip | GRAM_PAGE(1);
    *bank_reg = banksMirror;
    vram[WIDTH] = 8;
    vram[HEIGHT] = 8;
    for(r = 8; r < 120; r+=8) {
        vram[VY] = r;
        for(c = 0; c < 128; c+=8) {
            if(field[i]) {
                vram[VX] = c;
                vram[GX] = (field[i] & 0x0F) << 3;
                vram[GY] = (field[i] & 0xF0) >> 1;
                vram[START] = 1;
                wait();
            }
            ++i;
        }
    }
}

int main () {

    init_graphics();
    init_dynawave();
    init_music();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);            

    load_spritesheet(&ASSET__gfx__tinychars_bmp, 0);
    load_spritesheet(&ASSET__gfx__tiles_bmp, 1);
    load_spritesheet(&ASSET__gfx__title_bmp, 2);

    init_player();
    
    change_rom_bank(ASSET__maps__microban_slc_bank);
    next_level = (&ASSET__maps__microban_slc_ptr);
    player_select = 0;
    load_next_level();

    update_inputs();

    play_song(&ASSET__mid__yeeee_mid, REPEAT_NONE);

    while(!(player1_buttons & INPUT_MASK_START)) {
        clear_screen(243);
        draw_sprite(0,0,127,127,0,0,2);
        draw_sprite_frame(&ASSET__gfx__tinychars_json,
        64, 104,(player_select << 2) + 2 + ((tick & 8) >> 3), 0, 0);
        await_draw_queue();
        sleep(1);
        flip_pages();
        update_inputs();
        tick_music();
        ++tick;
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
            --player_select;
            if(player_select == 0xFF) {
                player_select = 5;
            }
        }
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
            player_select = (player_select+1)%6;
        }
    }

    stop_music();

    while (1) {                                     //  Run forever
        clear_screen(243);
        if(anim_timer) {
            anim_frame = 2 + ((anim_timer & 4) >> 2);
            if((tick & 1) || (player1_buttons & INPUT_MASK_B)) {
                offset_x += move_x;
                offset_y += move_y;
                --anim_timer;
            }
            if(!anim_timer) {
                player_x += move_x;
                player_y += move_y;
                if(pushing_box) {
                    i = (player_x + move_x) | ((player_y + move_y) << 4);
                    if(field[i] == GOAL_TILE) {
                        field[i] = BARREL_GOAL_TILE;
                        --goals_remaining;
                    }
                    else {
                        field[i] = BARREL_TILE;
                    }
                    pushing_box = 0;
                }
                move_x = 0;
                move_y = 0;
                offset_x = DEFAULT_OFFSET;
                offset_y = DEFAULT_OFFSET;
            }
        } else {
            anim_frame = ((tick & 32) >> 5);
        }

        anim_frame += anim_dir;
        anim_frame += player_select << 2;

        update_inputs();

       

        if(anim_timer == 0) {
            if(player1_buttons & INPUT_MASK_LEFT) {
                move_x = 0xFF;
                anim_timer = ANIM_TIME;
                anim_dir = 0;
                anim_flip = 0;
            }
            else if(player1_buttons & INPUT_MASK_RIGHT) {
                move_x = 1;
                anim_timer = ANIM_TIME;
                anim_dir = 0;
                anim_flip = SPRITE_FLIP_X;
            }
            else if(player1_buttons & INPUT_MASK_UP) {
                move_y = 0xFF;
                anim_timer = ANIM_TIME;
                anim_dir = 48;
                anim_flip = 0;
            }
            else if(player1_buttons & INPUT_MASK_DOWN) {
                move_y = 1;
                anim_timer = ANIM_TIME;
                anim_dir = 24;
                anim_flip = 0;
            }

            if(anim_timer == ANIM_TIME) {
                i = (player_x + move_x) | ((player_y + move_y) << 4);
                if(field[i] & 128) {
                    if((field[i] == BARREL_TILE) || (field[i] == BARREL_GOAL_TILE)) {
                        c = (player_x + move_x + move_x) | ((player_y + move_y + move_y) << 4);
                        if(field[c] & 128) {
                            move_x = 0;
                            move_y = 0;
                            anim_timer = 0;
                        } else {
                            pushing_box = 1;
                            do_noise_effect(64, 0xFF, ANIM_TIME);
                            if(field[i] == BARREL_GOAL_TILE) {
                                field[i] = GOAL_TILE;
                                ++goals_remaining;
                            } else {
                                if((field_original[i] == BARREL_TILE)
                                || (field_original[i] == BARREL_GOAL_TILE)) {
                                    field[i] = 0;
                                } else {
                                    field[i] = field_original[i];
                                }
                            }
                        }
                    } else {
                        move_x = 0;
                        move_y = 0;
                        anim_timer = 0;
                    }
                } else {
                    do_noise_effect(80,0,1);
                }
            }
        }

        while(queue_pending != 0) {
            asm("CLI");
            wait();
        }
        draw_field();

        if(goals_remaining == 0) {
            load_next_level();
            play_song(&ASSET__mid__solve_mid, REPEAT_NONE);
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
            next_level = current_level;
            load_next_level();
            play_song(&ASSET__mid__oops_mid, REPEAT_NONE);
        }

        if(pushing_box) {
            draw_sprite_now((player_x << 3) + offset_x + (move_x << 3) - DEFAULT_OFFSET,
                (player_y << 3) + offset_y + (move_y << 3) - DEFAULT_OFFSET, 8, 8, 16, 24, 1);
        }
        clear_border(0);
        draw_sprite_frame(&ASSET__gfx__tinychars_json,
        (player_x << 3) + offset_x,
        (player_y << 3) + offset_y,
        anim_frame, anim_flip, 0);
        sleep(1);
        flip_pages();
        tick_music();
        ++tick;
    }

  return (0);                                     //  We should never get here!
}