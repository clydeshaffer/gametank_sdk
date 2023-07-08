#include "gt/banking.h"

#include "dictionary.h"

#include "gen/assets/dict__index.h"
#include "gen/assets/dict__answers.h"
#include "gen/assets/dict_a.h"
#include "gen/assets/dict_b.h"
#include "gen/assets/dict_c.h"
#include "gen/assets/dict_d.h"
#include "gen/assets/dict_e.h"
#include "gen/assets/dict_f.h"
#include "gen/assets/dict_g.h"
#include "gen/assets/dict_h.h"
#include "gen/assets/dict_i.h"
#include "gen/assets/dict_j.h"
#include "gen/assets/dict_k.h"
#include "gen/assets/dict_l.h"
#include "gen/assets/dict_m.h"
#include "gen/assets/dict_n.h"
#include "gen/assets/dict_o.h"
#include "gen/assets/dict_p.h"
#include "gen/assets/dict_q.h"
#include "gen/assets/dict_r.h"
#include "gen/assets/dict_s.h"
#include "gen/assets/dict_t.h"
#include "gen/assets/dict_u.h"
#include "gen/assets/dict_v.h"
#include "gen/assets/dict_w.h"
#include "gen/assets/dict_x.h"
#include "gen/assets/dict_y.h"
#include "gen/assets/dict_z.h"

char dict_i;
char word_buf[WORD_LENGTH];
char secret_word[WORD_LENGTH+1];
char secret_letters[ALPHABET_SIZE];
char letters_seen[ALPHABET_SIZE];
const char qwerty_order[ALPHABET_SIZE] = {10, 23, 21, 12, 2, 13, 14, 15, 7, 16, 17, 18, 25, 24, 8, 9, 0, 3, 11, 4, 6, 22, 1, 20, 5, 19};

const char letter_banks[ALPHABET_SIZE] = {
    ASSET__dict_a__words_bin_bank,
    ASSET__dict_b__words_bin_bank,
    ASSET__dict_c__words_bin_bank,
    ASSET__dict_d__words_bin_bank,
    ASSET__dict_e__words_bin_bank,
    ASSET__dict_f__words_bin_bank,
    ASSET__dict_g__words_bin_bank,
    ASSET__dict_h__words_bin_bank,
    ASSET__dict_i__words_bin_bank,
    ASSET__dict_j__words_bin_bank,
    ASSET__dict_k__words_bin_bank,
    ASSET__dict_l__words_bin_bank,
    ASSET__dict_m__words_bin_bank,
    ASSET__dict_n__words_bin_bank,
    ASSET__dict_o__words_bin_bank,
    ASSET__dict_p__words_bin_bank,
    ASSET__dict_q__words_bin_bank,
    ASSET__dict_r__words_bin_bank,
    ASSET__dict_s__words_bin_bank,
    ASSET__dict_t__words_bin_bank,
    ASSET__dict_u__words_bin_bank,
    ASSET__dict_v__words_bin_bank,
    ASSET__dict_w__words_bin_bank,
    ASSET__dict_x__words_bin_bank,
    ASSET__dict_y__words_bin_bank,
    ASSET__dict_z__words_bin_bank
};

void set_lower_case() {
    if(word_buf[0] < 'a') word_buf[0] += ('a' - 'A');
    if(word_buf[1] < 'a') word_buf[1] += ('a' - 'A');
    if(word_buf[2] < 'a') word_buf[2] += ('a' - 'A');
    if(word_buf[3] < 'a') word_buf[3] += ('a' - 'A');
    if(word_buf[4] < 'a') word_buf[4] += ('a' - 'A');
}

void word_to_buf(char* word) {
    word_buf[4] = word[0];
    word_buf[3] = word[1];
    word_buf[2] = word[2];
    word_buf[1] = word[3];
    word_buf[0] = word[4];
}

char lookup_word(char* word) {
    char letter_bank;
    unsigned int left, right, mid;
    unsigned long target;
    long* arr;
    
    arr = (unsigned long*) &ASSET__dict_a__words_bin_ptr;
    
    word_to_buf(word);
    set_lower_case();
    
    change_rom_bank(ASSET__dict__index__counts_bin_bank);
    left = 0;
    right = ((unsigned int*)&ASSET__dict__index__counts_bin_ptr)[word_buf[4] - 'a'] - 1;
    mid = (left + right) / 2;

    letter_bank = letter_banks[word_buf[4] - 'a'];
    
    target = *((unsigned long*) (word_buf));
    change_rom_bank(letter_bank);

    while(left <= right) {
        if(arr[mid] < target)
            left = mid + 1;
        else if(arr[mid] == target)
            return 1;
        else
            right = mid - 1;

        mid = (left + right) / 2;
    }

    return 0;
}


char* word_ptr;
void set_secret_word(unsigned int index) {
#ifdef HARD_MODE
    long le_suffix;
    change_rom_bank(ASSET__dict__index__counts_bin_bank);
    dict_i = 0;
    while(index > ((unsigned int*)&ASSET__dict__index__counts_bin_ptr)[dict_i]) {
        index -= ((unsigned int*)&ASSET__dict__index__counts_bin_ptr)[dict_i];
        ++dict_i;
        if(dict_i >= ALPHABET_SIZE) {
            dict_i = 0;
        }
    }
    secret_word[0] = dict_i + 'A';
    change_rom_bank(letter_banks[dict_i]);
    le_suffix = ((unsigned long*) &ASSET__dict_a__words_bin_ptr)[index];
    word_to_buf(&le_suffix);
    *((long*)(secret_word+1)) = *((long*)(word_buf+1)) - 0x20202020;
    
#else
    while(index > 2309) {
        index -= 2309;
    }
    index = (index << 2) + index;
    word_ptr = &ASSET__dict__answers__answerlist_bin_ptr;
    word_ptr += index;
    change_rom_bank(ASSET__dict__answers__answerlist_bin_bank);
    secret_word[0] = word_ptr[0] - 0x20;
    secret_word[1] = word_ptr[1] - 0x20;
    secret_word[2] = word_ptr[2] - 0x20;
    secret_word[3] = word_ptr[3] - 0x20;
    secret_word[4] = word_ptr[4] - 0x20;
#endif

    secret_word[5] = 0;

    for(dict_i = 0; dict_i < ALPHABET_SIZE; ++dict_i) {
        secret_letters[dict_i] = 0;
    }
    ++secret_letters[secret_word[0]-'A'];
    ++secret_letters[secret_word[1]-'A'];
    ++secret_letters[secret_word[2]-'A'];
    ++secret_letters[secret_word[3]-'A'];
    ++secret_letters[secret_word[4]-'A'];
}

const char* get_secret_word() {
    return secret_word;
}

char check_guess(char* word, char* colors, char* keyb_colors) {
    for(dict_i = 0; dict_i < ALPHABET_SIZE; ++dict_i) {
        letters_seen[dict_i] = 0;
    }
    for(dict_i = 0; dict_i < WORD_LENGTH; ++dict_i) {
        if(*word == secret_word[dict_i]) {
             ++letters_seen[(*word) - 'A'];
            *colors = LETTER_COLOR_RIGHT;
            keyb_colors[qwerty_order[(*word) - 'A']] = LETTER_COLOR_RIGHT;
        }
        ++word;
        ++colors;
    }
    word -= WORD_LENGTH;
    colors -= WORD_LENGTH;
    for(dict_i = 0; dict_i < WORD_LENGTH; ++dict_i) {
        ++letters_seen[(*word) - 'A'];
        if(*word == secret_word[dict_i]) {
        } else if(letters_seen[(*word) - 'A'] <= secret_letters[(*word) - 'A']) {
            *colors = LETTER_COLOR_MOVED;
            if(keyb_colors[qwerty_order[(*word) - 'A']] != LETTER_COLOR_RIGHT) {
                keyb_colors[qwerty_order[(*word) - 'A']] = LETTER_COLOR_MOVED;
            }
        }
        if(!secret_letters[(*word) - 'A']) {
            keyb_colors[qwerty_order[(*word) - 'A']] = LETTER_COLOR_WRONG;
        }
        ++word;
        ++colors;
    }
    if(*(--colors) != LETTER_COLOR_RIGHT) return 0;
    if(*(--colors) != LETTER_COLOR_RIGHT) return 0;
    if(*(--colors) != LETTER_COLOR_RIGHT) return 0;
    if(*(--colors) != LETTER_COLOR_RIGHT) return 0;
    if(*(--colors) != LETTER_COLOR_RIGHT) return 0;
    return 1;
}