#ifndef MUSIC_H
#define MUSIC_H

#define REPEAT_NONE 0
#define REPEAT_LOOP 1
#define REPEAT_RESUME 2

void init_music();

void play_song(const unsigned char* song, char bank_num, char loop);

void tick_music();

void do_noise_effect(char note, char bend, char duration);

void stop_music();

void pause_music();

void unpause_music();

void set_note(char ch, char n);

#define NUM_FM_CHANNELS 4
#define NUM_FM_OPS 16

extern unsigned char audio_amplitudes[NUM_FM_OPS];

#endif