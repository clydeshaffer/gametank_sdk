#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include "audio_coprocessor.h"

typedef struct {
    unsigned char env_initial[OPS_PER_CHANNEL];
    unsigned char env_decay[OPS_PER_CHANNEL];
    unsigned char env_sustain[OPS_PER_CHANNEL];
    unsigned char op_transpose[OPS_PER_CHANNEL];
    unsigned char feedback;
    signed char transpose;
} Instrument;

#define INSTR_IDX_PIANO 1
#define INSTR_IDX_GUITAR 2
#define INSTR_IDX_DISTGUITAR 3
#define INSTR_IDX_SLAPBASS 4
#define INSTR_IDX_SNARE 5
#define INSTR_IDX_SITAR 6
#define INSTR_IDX_HORN 7

Instrument* get_instrument_ptr(char idx);

#endif