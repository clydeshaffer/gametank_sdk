#ifndef DICTIONARY_H
#define DICTIONARY_H

#define WORD_LENGTH 5
#define GUESS_LIMIT 6
#define ALPHABET_SIZE 26

char lookup_word(char* word);

void set_secret_word(unsigned int index);

const char* get_secret_word();

char check_guess(char* word, char* colors);

#endif