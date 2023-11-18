/*
 * Provides random numbers
 */

#ifndef RANDOM_H
#define RANDOM_H

void set_rnd_seed();

int rnd();
int weak_rnd();

int rnd_range(int low, int high);

#endif
