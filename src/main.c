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
int audio_data_counter;

#define CARD_COUNT 52
#define DECK_SIZE 52
#define START_HAND_SIZE 0
char hand_size;
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
            audio_data_counter = ASSET__audiodata__##x##_bin_size;

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

char select_cursor, cardgap, turn, dealing, wait_timer, selecting, requesting, requested;

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
    dealing = 7;
    wait_timer = 30;
    selecting = 0;
    requesting = 0xFF;

    card_x[0] = 64;
    for(i = 0; i < START_HAND_SIZE; ++i) {
        card_y[i] = UNSELECT_HEIGHT;
        card_vx[i] = 0;
        card_vy[i] = 0;
        card_num[i] = deck[i];
        deck[i] = 0x00;
        ++deck_index;
    }
    hand_size = START_HAND_SIZE;
    cardgap = 14;

    for(i = START_HAND_SIZE; i < CARD_COUNT; ++i) {
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
            }
        }

        cardgap = 21 - hand_size;
        if(hand_size > 20) cardgap = 1;
        card_vx[0] = (hand_size < HAND_OFFSETS_COUNT) ? hand_offset[hand_size] : hand_offset[HAND_OFFSETS_COUNT-1];
        if(card_x[0] > card_vx[0]) --card_x[0];
        else if(card_x[0] < card_vx[0]) ++card_x[0];
        card_vx[0] = 0;
        
        for(i = CARD_COUNT - 1; i != 255; --i) {
            if(card_num[i] != 0xFF) {
                if(i < hand_size) {
                    if(i) {
                        card_x[i] = card_x[i-1] + cardgap;
                    }
                    if((i == select_cursor) && selecting) {
                        if(card_y[i] > SELECT_HEIGHT) card_y[i] -= 2;
                    } else {
                        if(card_y[i] < UNSELECT_HEIGHT) card_y[i] += 2;
                    }

                    if(card_y[i] > 196) {
                        card_num[i] = 0xFF;
                    }
                }

                draw_sprite(card_x[i], card_y[i], 16, 24, (card_num[i] & 15) << 4, suit_offset[card_num[i] >> 4], bankflip | BANK_CLIP_X | BANK_CLIP_Y);
                card_x[i] += card_vx[i];
                card_y[i] += card_vy[i];
                if(card_x[i] == 1) {
                    card_vx[i] = 1;
                } else if(card_x[i] == 119) {
                    card_vx[i] = -1;
                }
                if(card_y[i] == 8) {
                    card_vy[i] = 1;
                } else if(card_y[i] == 112) {
                    card_vy[i] = -1;
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
                card_vy[select_cursor] = -3;
                setclip(thankyou);
                selecting = 0;
            } else {
                setclip(no);
            }
        }

        if(wait_timer) {
            --wait_timer;
            if(!wait_timer) {
                if(dealing) {
                    card_x[hand_size] = 64;
                    card_num[hand_size] = deck[deck_index++];
                    card_y[hand_size] = 72;
                    ++hand_size;
                    --dealing;
                    wait_timer = DEAL_ANIM_DELAY;
                } else if (!selecting && !audio_data_counter && (requesting = 0xFF)) {
                    setclip(giveme);
                    requesting = 8;//rnd_range(0,13);
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