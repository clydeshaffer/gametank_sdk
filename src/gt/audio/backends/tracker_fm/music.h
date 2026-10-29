#ifndef MUSIC_H
#define MUSIC_H

#include "../../audio_coprocessor.h"

#define REPEAT_NONE 0
#define REPEAT_LOOP 1
#define REPEAT_RESUME 2

#define SFX_PRIORITY(x) (x<<4)

typedef struct channel_header_t {
    char patternCount;
    char sequenceLength;
    char* patternHeads[];
} channel_header_t;

typedef struct instrument_header_t {
    char params[4];
    char* envelopes[4];
} instrument_header_t;

typedef struct track_header_t {
    char track_fmt_version;
    char step_frames;
    char pattern_length;
    char reserved[4];
    channel_header_t* channel_datas[4];
    char instrument_count;
    instrument_header_t* instruments[];
} track_header_t;

void init_music();

void init_song_data();

void play_song(const track_header_t* song, char bank_num, char loop);

void tick_music();

void silence_all_channels();

void stop_music();

void pause_music();

void unpause_music();

void set_note(char ch, char n);

extern unsigned char audio_amplitudes[NUM_FM_OPS];

void load_instrument(char channel, char instrumentIdx);

void play_sound_effect(char sfx_id, char channel);

void play_boot_jingle();

#endif