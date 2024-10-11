#ifndef BYTESINE_H
#define BYTESINE_H

#define SINE_SCALE 8

extern const int sine_table[256];
extern const int cosine_table[256];
extern const unsigned char arccos_table[256];

long check_cosine_sum();

#endif