#include "../gt/gametank.h"
#include "../gt/input.h"
#include "../gt/dynawave.h"
#include "../gt/music.h"
#include "../gt/drawing_funcs.h"
#include "../gt/banking.h"
#include "../gt/random.h"
#include "tetris_logic.h"
#include "tetris_draw.h"
#include "../gen/assets/music.h"

#include "../gen/assets/tetris.h"

void breakpoint() {}

void Sleep(int frames) {
    int i;
    for(i = 0; i < frames; i++) {
        frameflag = 1;
        while(frameflag) {}
    }
}

#define GAME_STATE_TITLE 0
#define GAME_STATE_PLAY_SINGLE 2
#define GAME_STATE_PLAY_DUEL 3
#define GAME_STATE_SINGLE_END 4
#define GAME_STATE_DUEL_END 5
#define STATE_FLAG_END 4
#define STATE_FLAG_DUEL 1

extern char auto_tick_music;

extern void wait();
char i;
char did_init_music = 0;
char music_cnt = 0;
char game_state = GAME_STATE_TITLE;
char mode_select = 0;
int tetris_main() {

    init_graphics();
    init_dynawave();
    init_music();
    auto_tick_music = 1;

    load_spritesheet(&ASSET__tetris__gamesprites_bmp, 0);
    load_spritesheet(&ASSET__tetris__title_bmp, 1);
    load_spritesheet(&ASSET__tetris__bg_bmp, 2);
    init_tetromino_minis();

    draw_box_now(0, SCREEN_HEIGHT-1, SCREEN_WIDTH - 1, 1, 32);
    wait();
    draw_box_now(0, SCREEN_HEIGHT-1, SCREEN_WIDTH - 1, 1, 32);
    wait();
    clear_border(32);
    await_draw_queue();
    flip_pages();
    clear_border(32);
    await_draw_queue();

    
    players[0].playernum = 0;
    players[0].playField = playField_0;
    players[0].field_offset_x = 16;
    players[0].field_offset_y = 24;

    players[1].playernum = 1;
    players[1].playField = playField_1;
    players[1].field_offset_x = 72;
    players[1].field_offset_y = 24;

    while(1){
        update_inputs();
        
        if(game_state == GAME_STATE_TITLE) {
            draw_sprite_now(0, 0, 127, 127, 0, 0, 1);
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                if(mode_select)
                    game_state = GAME_STATE_PLAY_DUEL;
                else
                    game_state = GAME_STATE_PLAY_SINGLE;
                music_cnt = 0;
                play_song(&ASSET__music__pizza_mid, REPEAT_LOOP);
                for(i = 0; i < 220; ++i) {
                    playField_0[i] = 0;
                    playField_1[i] = 0;
                }
                change_rom_bank(0xFE);
                initPlayerState(&(players[0]));
                initPlayerState(&(players[1]));
            }
            if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_UP | INPUT_MASK_DOWN)) {
                mode_select ^= 16;
            }
            rnd();
            wait();
            draw_sprite_now(23, 73 + mode_select, 8, 7, 88, 113, 0);
        } else {
            draw_sprite_now(0, 0, 127, 127, 0, 0, 2);

            via[ORB] = 0x80;
            via[ORB] = 0x03;

            if(game_state & STATE_FLAG_END) {
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    game_state = GAME_STATE_TITLE;    
                } else if((game_state & STATE_FLAG_DUEL) && (player2_buttons & ~player2_old_buttons & INPUT_MASK_START)) {
                    game_state = GAME_STATE_TITLE;    
                }
                if(game_state == GAME_STATE_TITLE) {
                    change_rom_bank(0xFE);
                    initPlayerState(&(players[0]));
                    initPlayerState(&(players[1]));
                }
            } else {
                change_rom_bank(0xFE);
                players[1].pendingGarbage += updatePlayerState(&(players[0]), player1_buttons, player1_old_buttons);
                if(game_state & STATE_FLAG_DUEL)
                    players[0].pendingGarbage += updatePlayerState(&(players[1]), player2_buttons, player2_old_buttons);
            }

            if((players[0].flags | players[1].flags) & PLAYER_DEAD) {
                game_state |= STATE_FLAG_END;
                did_init_music = 0;
                stop_music();
            }
            via[ORB] = 0x80;
            via[ORB] = 0x43;
            wait();
            *bank_reg = bankflip;
            via[ORB] = 0x80;
            via[ORB] = 0x00;
            drawPlayerState(&(players[0]));
            if(game_state & STATE_FLAG_DUEL)
                drawPlayerState(&(players[1]));

            
            via[ORB] = 0x80;
            via[ORB] = 0x40;
        }
        flip_pages();
        Sleep(1);
    }
}