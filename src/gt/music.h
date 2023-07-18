#ifndef MUSIC_H
#define MUSIC_H

#define REPEAT_NONE 0
#define REPEAT_LOOP 1
#define REPEAT_RESUME 2

#define SONG_STATUS_NOCHANGE 0
#define SONG_STATUS_ENDED 1
#define SONG_STATUS_LOOPED 2

void init_music();

void play_song(const unsigned char* song, char bank_num, char loop);

char tick_music();

void do_noise_effect(char note, char bend, char duration);

void do_tone_effect(char channel, char note, char bend, char duration);

void stop_music();

void pause_music();

void unpause_music();

#endif