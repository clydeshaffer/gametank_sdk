#ifndef MUSIC_H
#define MUSIC_H

#include "audio_coprocessor.h"
#include "instruments.h"

#define REPEAT_NONE 0
#define REPEAT_LOOP 1
#define REPEAT_RESUME 2

#define SFX_PRIORITY(x) (x<<4)

void init_music();

void play_song(const unsigned char* song, char bank_num, char loop);

void tick_music();

void silence_all_channels();

void stop_music();

void pause_music();

void unpause_music();

void set_note(char ch, char n);

extern unsigned char audio_amplitudes[NUM_FM_OPS];

void load_instrument(char channel, const Instrument* instr);

void play_sound_effect(char sfx_id, char channel);

#endif