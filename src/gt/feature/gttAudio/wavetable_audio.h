#ifndef WAVETABLE_AUDIO_H
#define WAVETABLE_AUDIO_H

#include "gttAudio.h"

#define VOICE_BASE 0x3041
#define VOICE_SIZE 7
#define VOICE_COUNT 7

#define WAVETABLE_SIZE 256
#define WAVETABLE_COUNT 11

/* Instrument slot for Noise */
#define NOISE_INSTRUMENT 10

#define WAVETABLE_SAMPLE_RATE_REG 0xD7

typedef struct {
    unsigned int phase;
    unsigned int frequency;
    unsigned int wavetable;
    unsigned char volume;
} Voice;

#define VOICES ((Voice *) VOICE_BASE)

extern const unsigned int WAVETABLE[WAVETABLE_COUNT];

/* Reserved values to set the noise mode through the wavetable API */
#define NOISE_SENTINEL_MODE0 0xfffe
#define NOISE_SENTINEL_MODE1 0xffff

void init_wavetable_audio();

void load_wavetable_instrument(char slot, const unsigned char *table);

void mute_voice(char voice);

void mute_all_voices();

#endif
