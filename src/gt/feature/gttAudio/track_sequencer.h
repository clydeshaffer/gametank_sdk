#ifndef TRACK_SEQUENCER_H
#define TRACK_SEQUENCER_H

#include "gttAudio.h"

typedef struct {
    const unsigned char *track;
    unsigned char track_bank;
    unsigned char beat;
    unsigned int frame_acc;
    unsigned int tick_acc;
    unsigned int bpm;
    unsigned int speed;
    unsigned char pattern_idx;
    unsigned char pattern_count;
    unsigned char flow_count;
    unsigned char stopped;
    unsigned char fx_active_any;
} TrackSequencer;

void track_sequencer_init(TrackSequencer *seq, const unsigned char *track, unsigned char track_bank);

void track_sequencer_init_voices();

void track_sequencer_tick(TrackSequencer *seq);

#endif
