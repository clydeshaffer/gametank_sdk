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

extern unsigned char audio_amplitudes[16];

#endif