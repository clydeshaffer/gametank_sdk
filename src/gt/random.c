#include "random.h"

int xorshift16(int x) {
    x |= x == 0;   /* if x == 0, set x = 1 instead */
    x ^= (x & 0x07ff) << 5;
    x ^= x >> 7;
    x ^= (x & 0x0003) << 14;
    return x;
}

int rnd_seed = 234;

int rnd() {
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    rnd_seed = xorshift16(rnd_seed);
    return rnd_seed;
}

int rnd_range(int low, int high) {
    return ((rnd() & 0x7FFF) % (high - low)) + low;
}