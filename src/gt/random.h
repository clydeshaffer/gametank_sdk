/*
 * Provides random numbers
 */

#ifndef RANDOM_H
#define RANDOM_H

extern int rnd_seed;

int rnd();

int rnd_range(int low, int high);

#endif