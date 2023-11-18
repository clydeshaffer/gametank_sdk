#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "gen/assets/gfx.h"
#include "gen/assets/maps.h"
#include <zlib.h>
#include "banking.h"
#include "gen/assets/mid.h"
#include "dynawave.h"
#include "random.h"
#include "music.h"
#include "undo.h"

#define DEFAULT_OFFSET 4
#define ANIM_TIME 8

// TILES
#define BARREL_TILE 252
#define BARREL_GOAL_TILE 253
#define GOAL_TILE 16
#define PLAYER_START 32
#define PLAYER_GOAL_START 24

#define FIELD_SIZE 256

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
char pulling_box;
char pushing_box_pos;

char field[FIELD_SIZE];
char field_original[FIELD_SIZE];
char attempted_move_dir;
char c,r,i;
char* next_level;
char* current_level;

char kcode_pos = 0;
char noclip = 0;

char rand;


const short kcode_list[] = {
    0,
    INPUT_MASK_UP,
    0,
    INPUT_MASK_UP,
    0,
    INPUT_MASK_DOWN,
    0,
    INPUT_MASK_DOWN,
    0,
    INPUT_MASK_LEFT,
    0,
    INPUT_MASK_RIGHT,
    0,
    INPUT_MASK_LEFT,
    0,
    INPUT_MASK_RIGHT,
    0,
    INPUT_MASK_B,
    0,
    INPUT_MASK_A,
    0,
    INPUT_MASK_START,
};

#define MOVE_MASK 3

enum move_direction {
    move_left = 0,
    move_right = 1,
    move_up = 2,
    move_down = 3,
};

enum was_barrel_pushed {
    barrel_wasnt_pushed = 0,
    barrel_was_pushed = 4,
};

void scan_level() {
    i = 0;
    do {
        if((field[i] == PLAYER_START) || (field[i] == PLAYER_GOAL_START)) {
            player_x = i & 15;
            player_y = i >> 4;
            // NOTE this does change the behavior if a level previously had multiple starts
            // Should have a sort of lint for that sort of level though
            return;
        }
        ++i;
    } while(i);
}

void randomize_grass() {
    set_rnd_seed(current_level);
    i = 0;
    do {
        if(field[i] == 0) {
            // NOTE using this bc it's much faster and we don't care all that much
            rand = (char)weak_rnd();
            // Are the low two bits set of the top nibble clear?
            // This should happen ~25% of the time
            // If so let's decorate the grass
            if ((rand & 0x30) == 0) {
                rand %= 5;
                field[i] = 64 + rand;
                field_original[i] = 64 + rand;
            }
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
    pulling_box = 0;

    reset_undo();
}

void load_next_level() {
    change_rom_bank(ASSET__maps__microban_slc_bank);
    inflatemem(field, next_level+1);
    current_level = next_level;
    next_level += *next_level;
    next_level += 1;
    init_player();
    scan_level();
    randomize_grass();
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

void move_barrel_off_of(char i) {
    if(field[i] == BARREL_GOAL_TILE) {
        field[i] = GOAL_TILE;
    } else if((field_original[i] == BARREL_TILE)) {
        field[i] = 0;
    } else {
        field[i] = field_original[i];
    }
}

void main_menu_loop() {
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

char exec_kcode() {
    if (player1_buttons == kcode_list[kcode_pos+1]) {
        if (player1_buttons == INPUT_MASK_START) {
            // The start is the last btn in the chain
            // If we've gotten here we're ready to get funky
            kcode_pos = 0;
            noclip = 1;
            play_song(&ASSET__mid__yeeee_mid, REPEAT_NONE);
            return 1;
        }
        kcode_pos++;
    } else if (player1_buttons != kcode_list[kcode_pos]) {
        kcode_pos = 0;
    }

    return 0;
}

void game_loop() {
    update_inputs();
    if (exec_kcode()) return;

    clear_screen(243);
    if(anim_timer) {
        anim_frame = 2 + ((anim_timer & 4) >> 2);
        if((tick & 1) || (player1_buttons & INPUT_MASK_B)) {
            offset_x += move_x;
            offset_y += move_y;
            --anim_timer;
        }
        // Have we just now finished our animation?
        if(!anim_timer) {
            player_x += move_x;
            player_y += move_y;
            if(pushing_box) {
                i = (player_x + move_x) | ((player_y + move_y) << 4);
                if(field[i] == GOAL_TILE) {
                    field[i] = BARREL_GOAL_TILE;
                }
                else {
                    field[i] = BARREL_TILE;
                }
                pushing_box = 0;
            }
            // TODO lazy copy and paste
            if(pulling_box) {
                i = (player_x - move_x) | ((player_y - move_y) << 4);
                if(field[i] == GOAL_TILE) {
                    field[i] = BARREL_GOAL_TILE;
                }
                else {
                    field[i] = BARREL_TILE;
                }
                pulling_box = 0;
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

    if(anim_timer == 0) {
        if(player1_buttons & INPUT_MASK_C) {
            anim_timer = ANIM_TIME;
            move_x = 0;
            move_y = 0;
        } else if(player1_buttons & INPUT_MASK_LEFT) {
            attempted_move_dir = move_left;
            move_x = 0xFF;
            anim_timer = ANIM_TIME;
            anim_dir = 0;
            anim_flip = 0;
        } else if(player1_buttons & INPUT_MASK_RIGHT) {
            attempted_move_dir = move_right;
            move_x = 1;
            anim_timer = ANIM_TIME;
            anim_dir = 0;
            anim_flip = SPRITE_FLIP_X;
        } else if(player1_buttons & INPUT_MASK_UP) {
            attempted_move_dir = move_up;
            move_y = 0xFF;
            anim_timer = ANIM_TIME;
            anim_dir = 48;
            anim_flip = 0;
        } else if(player1_buttons & INPUT_MASK_DOWN) {
            attempted_move_dir = move_down;
            move_y = 1;
            anim_timer = ANIM_TIME;
            anim_dir = 24;
            anim_flip = 0;
        }

        if(anim_timer == ANIM_TIME) {
            if(player1_buttons & INPUT_MASK_C) {
                char last_move = undo_buffer_pop();
                if (last_move != 0xFF) {
                    // Reverse movement direction
                    if ((last_move & MOVE_MASK) == move_left)
                        move_x = 1;
                    else if ((last_move & MOVE_MASK) == move_right)
                        move_x = 0xFF;
                    else if ((last_move & MOVE_MASK) == move_up)
                        move_y = 1;
                    else if ((last_move & MOVE_MASK) == move_down)
                        move_y = 0xFF;

                    // If the barrel was moved take care of that
                    if (last_move & barrel_was_pushed) {
                        i = (player_x - move_x) | ((player_y - move_y) << 4);
                        move_barrel_off_of(i);
                        pulling_box = 1;
                    }

                    do_noise_effect(40, 0xFF, ANIM_TIME);
                }
            } else {
                i = (player_x + move_x) | ((player_y + move_y) << 4);
                if(field[i] & 128) {
                    // Attempting to push a barrel
                    if((field[i] == BARREL_TILE) || (field[i] == BARREL_GOAL_TILE)) {
                        // Target tile for barrel
                        c = (player_x + move_x + move_x) | ((player_y + move_y + move_y) << 4);
                        if(field[c] & 128) {
                            // Attempting to push a barrel into a wall or another barrel
                            if (!noclip) {
                                move_x = 0;
                                move_y = 0;
                                anim_timer = 0;
                            } else {
                                undo_buffer_push(attempted_move_dir | barrel_wasnt_pushed);
                                do_noise_effect(120,2,2);
                            }
                        } else {
                            // Move and push barrel
                            undo_buffer_push(attempted_move_dir | barrel_was_pushed);
                            pushing_box = 1;
                            do_noise_effect(64, 0xFF, ANIM_TIME);
                            move_barrel_off_of(i);
                        }
                    } else {
                        // Attempting to move into a wall
                        if (!noclip) {
                            move_x = 0;
                            move_y = 0;
                            anim_timer = 0;
                        } else {
                            undo_buffer_push(attempted_move_dir | barrel_wasnt_pushed);
                            do_noise_effect(120,2,2);
                        }
                    }
                } else {
                    // Simple move
                    undo_buffer_push(attempted_move_dir | barrel_wasnt_pushed);
                    do_noise_effect(80,0,1);
                }
            }
        }
    }

    while(queue_pending != 0) {
        asm("CLI");
        wait();
    }
    draw_field();

    if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
        next_level = current_level;
        load_next_level();
        play_song(&ASSET__mid__oops_mid, REPEAT_NONE);
    }

    if(pushing_box) {
        pushing_box_pos = (player_x + move_x) | ((player_y + move_y) << 4);
        draw_sprite_now((player_x << 3) + offset_x + (move_x << 3) - DEFAULT_OFFSET,
            (player_y << 3) + offset_y + (move_y << 3) - DEFAULT_OFFSET, 8, 8, 16, 24, 1);
    }

    if(pulling_box) {
        draw_sprite_now((player_x << 3) + offset_x - (move_x << 3) - DEFAULT_OFFSET,
            (player_y << 3) + offset_y - (move_y << 3) - DEFAULT_OFFSET, 8, 8, 16, 24, 1);
    }

    // Check for level completion
    // I made this slower so that I didn't have to keep track of how many
    // goals were remaining lol
    for (i = 0; i < FIELD_SIZE; i++) {
        // If we find any GOAL_TILEs the level isn't solved
        // Otherwise they would be BARREL_GOAL_TILEs
        if (field[i] == GOAL_TILE && !(pushing_box && (pushing_box_pos == i))) break;
        // We've finished looking for GOAL_TILEs and haven't found any
        // The level is complete!
        if (i == FIELD_SIZE - 1) {
            load_next_level();
            play_song(&ASSET__mid__solve_mid, REPEAT_NONE);
        }
    }

    clear_border(0);
    draw_sprite_frame(
        &ASSET__gfx__tinychars_json,
        (player_x << 3) + offset_x,
        (player_y << 3) + offset_y,
        anim_frame, anim_flip,
        0);
    sleep(1);
    flip_pages();
    tick_music();
    ++tick;
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

    // Start menu
    while(!(player1_buttons & INPUT_MASK_START)) {
        main_menu_loop();
    }

    stop_music();

    // Main game loop, run forever
    while (1) {
        game_loop();
    }

  // We should never get here!
  return (0);
}
