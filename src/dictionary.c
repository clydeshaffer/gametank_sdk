#include "gt/banking.h"

#include "dictionary.h"

#include "gen/assets/dict__index.h"
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

char word_buf[5];

const char letter_banks[26] = {
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