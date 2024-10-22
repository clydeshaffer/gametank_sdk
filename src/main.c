#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"
#include "dynawave.h"
#include "gen/assets/audiodata.h"
#include "gen/assets/img.h"
#include "random.h"

char* audio_data_cursor;
char* last_audio_cursor;
int audio_data_counter;

#define CARD_COUNT 52
#define DECK_SIZE 52
#define START_HAND_SIZE 7
char hand_size;
char cpu_hand_size; //CPU hand counts down from CARD_COUNT - 1
char cpu_hand_start_index;
char card_x[CARD_COUNT];
char card_y[CARD_COUNT];
char card_vx[CARD_COUNT];
char card_vy[CARD_COUNT];
char card_num[CARD_COUNT];
char deck[DECK_SIZE];
char deck_index;

#define SELECT_HEIGHT 86
#define UNSELECT_HEIGHT 104

#define DEAL_ANIM_DELAY 15

const char suit_offset[4] = {0, 24, 48, 72};

#define HAND_OFFSETS_COUNT 12
const char hand_offset[HAND_OFFSETS_COUNT] = { 0, 64, 48, 32, 25, 21, 18, 14, 12, 10, 8, 4 };

#define setclip(x) audio_data_cursor = &ASSET__audiodata__##x##_bin_ptr; \
            audio_data_counter = ASSET__audiodata__##x##_bin_size; \
            last_audio_cursor = audio_data_cursor;

void set_voice_clip(char x) {
    switch (x) {
        case 1:
            setclip(ace);
            break;
        case 2:
            setclip(two);
            break;
        case 3:
            setclip(three);
            break;
        case 4:
            setclip(four);
            break;
        case 5:
            setclip(five);
            break;
        case 6:
            setclip(six);
            break;
        case 7:
            setclip(seven);
            break;
        case 8:
            setclip(eight);
            break;
        case 9:
            setclip(nine);
            break;
        case 10:
            setclip(ten);
            break;
        case 11:
            setclip(jacks);
            break;
        case 12:
            setclip(queen);
            break;
        case 13:
            setclip(king);
            break;
    }
}

void init_deck() {
    char i;
    deck_index = 0;
    deck[0] = 0x00;
    for(i = 1; i < DECK_SIZE; ++i) {
        deck[i] = deck[i-1] + 1;
        if((deck[i] & 0x0F) > 0x0C) {
            deck[i] = (deck[i] & 0xF0) + 0x10;
        }
    }
}

void shuffle_deck() {
    char i, swap, select;
    for(i = 0; i < DECK_SIZE; ++i) {
        swap = deck[i];
        select = rnd_range(i, DECK_SIZE);
        deck[i] = deck[select];
        deck[select] = swap;
    }
}

#define STATE_NONE 0
#define STATE_DEALING 1
#define STATE_CPU_ASKING 2
#define STATE_CPU_DRAWING 3
#define STATE_PLAYER_GIVING 4
#define STATE_PLAYER_ASKING 5
#define STATE_CPU_GIVING 6
#define STATE_PLAYER_DRAWING 7

char select_cursor, cardgap, turn, dealing, wait_timer, selecting, requesting, requested;
char game_state, timeout_state;

int main () {
    static char i, nextcard;

    init_graphics();

    load_spritesheet(&ASSET__img__cards_bmp, 0);
    load_spritesheet(&ASSET__img__cards_1_bmp, QUADRANT_1);

    init_dynawave();

    set_audio_param(PITCH_MSB+0, 0x00);
    set_audio_param(PITCH_LSB+0, 0x00);
    set_audio_param(PITCH_MSB+1, 0x00);
    set_audio_param(PITCH_LSB+1, 0x00);
    set_audio_param(PITCH_MSB+2, 0x00);
    set_audio_param(PITCH_LSB+2, 0x00);
    set_audio_param(PITCH_MSB+3, 0x00);
    set_audio_param(PITCH_LSB+3, 0x00);
    set_audio_param(PITCH_MSB+4, 0x00);
    set_audio_param(PITCH_LSB+4, 0x00);
    set_audio_param(PITCH_MSB+5, 0x00);
    set_audio_param(PITCH_LSB+5, 0x00);
    set_audio_param(PITCH_MSB+6, 0x00);
    set_audio_param(PITCH_LSB+6, 0x00);
    set_audio_param(PITCH_MSB+7, 0x00);
    set_audio_param(PITCH_LSB+7, 0x00);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    init_deck();
    shuffle_deck();
    dealing = START_HAND_SIZE;
    wait_timer = 30;
    selecting = 0;
    requesting = 0xFF;

    hand_size = 0;
    cpu_hand_size = 0;
    cpu_hand_start_index = CARD_COUNT;
    cardgap = 14;

    for(i = 0; i < CARD_COUNT; ++i) {
        card_num[i] = 0xFF;
    }

    audio_data_cursor = &ASSET__audiodata__ace_bin_ptr;
    audio_data_counter = 0;
    nextcard = 0;

    while (1) {                                     //  Run forever
        clear_screen(11);

        if(audio_data_counter) {
            change_rom_bank(ASSET__audiodata__two_bin_bank);
            for(i = 0; i < 8; ++i) {
                push_audio_param(PITCH_LSB+i, *audio_data_cursor);
                ++audio_data_cursor;
                push_audio_param(PITCH_MSB+i, *audio_data_cursor);
                ++audio_data_cursor;
                audio_data_counter -= 2;
            }
            flush_audio_params();
            pop_rom_bank();
            if(!audio_data_counter) {
                for(i = 0; i < 8; ++i) {
                    push_audio_param(PITCH_LSB+i, 0);
                    push_audio_param(PITCH_MSB+i, 0);
                }
                flush_audio_params();
                if(requesting != 0xFF) {
                    set_voice_clip(requesting+1);
                    requested = requesting;
                    requesting = 0xFF;
                } else if(requested != 0xFF) {
                    selecting = 1;
                }

                if(last_audio_cursor == &ASSET__audiodata__awdangit_bin_ptr) {
                    setclip(giveme);
                    requesting = card_num[rnd_range(cpu_hand_start_index, CARD_COUNT)] & 0x0F;
                    requested = 0xFF;
                }
            }
        }

        cardgap = 21 - hand_size;
        if(hand_size > 20) cardgap = 1;
        card_vx[0] = (hand_size < HAND_OFFSETS_COUNT) ? hand_offset[hand_size] : hand_offset[HAND_OFFSETS_COUNT-1];
       
        for(i = 1; i < hand_size; ++i) {
            card_vx[i] = card_vx[i-1] + cardgap;
        }

        cardgap = 21 - cpu_hand_size;
        card_vx[cpu_hand_start_index] = ((cpu_hand_size < HAND_OFFSETS_COUNT) ? hand_offset[cpu_hand_size] : hand_offset[HAND_OFFSETS_COUNT-1]);
        for(i = cpu_hand_start_index+1; i < CARD_COUNT; ++i) {
            card_vx[i] = card_vx[i-1] + cardgap;
        }

        for(i = 0; i < CARD_COUNT; ++i) {
            if(card_num[i] != 0xFF) {
                if(i < hand_size) {
                    if(selecting) {
                        if(i == select_cursor) {
                            card_vy[i] = SELECT_HEIGHT;
                        } else {
                            card_vy[i] = UNSELECT_HEIGHT;
                        }
                    }
                }
                if(card_y[i] == 0) {
                    if(!(card_num[i] & 0x80)) {
                        card_num[i] |= 0x80;
                        --cpu_hand_start_index;
                        card_num[cpu_hand_start_index] = card_num[i];
                        card_num[i] = 0xFF;
                        ++cpu_hand_size;
                    }
                }

                if(card_x[i] > card_vx[i]) --card_x[i];
                else if(card_x[i] < card_vx[i]) ++card_x[i];
                if(card_y[i] > card_vy[i]) --card_y[i];
                else if(card_y[i] < card_vy[i]) ++card_y[i];
            }
        }

        for(i = CARD_COUNT - 1; i != 255; --i) {
            if(card_num[i] != 0xFF) {
                if(card_num[i] & 0x80) {
                    draw_sprite(card_x[i], card_y[i], 16, 24, 0, 96, bankflip | BANK_CLIP_X | BANK_CLIP_Y);
                } else {
                    draw_sprite(card_x[i], card_y[i], 16, 24, (card_num[i] & 15) << 4, suit_offset[card_num[i] >> 4], bankflip | BANK_CLIP_X | BANK_CLIP_Y);
                }
            }
        }        

        clear_border(0);
        
        if(select_cursor < hand_size-1) {
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
                ++select_cursor;
            }
        }
        if(select_cursor > 0) {
            if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
                --select_cursor;
            }
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
            //set_voice_clip((card_num[select_cursor] & 15) + 1);
            if((card_num[select_cursor] & 15) == requested) {
                card_x[hand_size] = card_x[select_cursor];
                card_y[hand_size] = card_y[select_cursor];
                card_vx[hand_size] = card_vx[select_cursor];
                card_vy[hand_size] = 0;
                card_num[hand_size] = card_num[select_cursor];
                card_num[select_cursor] = 0xFF;
                for(i = select_cursor; i < hand_size-1; ++i) {
                    card_num[i] = card_num[i+1];
                    card_x[i] = card_x[i+1];
                    card_y[i] = card_y[i+1];
                }
                --hand_size;
                card_num[hand_size] = 0xFF;
                setclip(thankyou);
                selecting = 0;
            } else {
                setclip(no);
            }
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_B) {
            if(requested != 0xFF) {
                setclip(awdangit);
            }
        }

        if(wait_timer) {
            --wait_timer;
            if(!wait_timer) {
                if(dealing) {
                    card_x[hand_size] = 64;
                    card_y[hand_size] = 64;
                    card_num[hand_size] = deck[deck_index++];
                    card_vy[hand_size] = UNSELECT_HEIGHT;
                    ++hand_size;

                    --cpu_hand_start_index;
                    card_x[cpu_hand_start_index] = 64;
                    card_y[cpu_hand_start_index] = 64;
                    card_num[cpu_hand_start_index] = deck[deck_index++] | 0x80;
                    card_vy[cpu_hand_start_index] = 0;
                    ++cpu_hand_size;

                    --dealing;
                    wait_timer = DEAL_ANIM_DELAY;
                } else if (!selecting && !audio_data_counter && (requesting = 0xFF)) {
                    setclip(giveme);
                    requesting = card_num[rnd_range(cpu_hand_start_index, CARD_COUNT)] & 0x0F;
                }
            }
        }

        await_draw_queue();
        sleep(1);
        flip_pages();
        update_inputs();

    }

  return (0);                                     //  We should never get here!
}