#ifndef DICTIONARY_H
#define DICTIONARY_H

#define WORD_LENGTH 5
#define GUESS_LIMIT 6
#define ALPHABET_SIZE 26

#define LETTER_COLOR_RIGHT 20
#define LETTER_COLOR_MOVED 52
#define LETTER_COLOR_WRONG 2
#define LETTER_COLOR_DEFAULT 3

char lookup_word(char* word);

void set_secret_word(unsigned int index);

const char* get_secret_word();

char check_guess(char* word, char* colors, char* keyb_colors);

#endif