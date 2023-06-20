#ifndef MUSIC_H
#define MUSIC_H

#define MUSIC_TRACK_NONE 0
#define MUSIC_TRACK_TITLE 1
#define MUSIC_TRACK_DIED 2
#define MUSIC_TRACK_STAIRS 3
#define MUSIC_TRACK_MAIN 4
#define MUSIC_TRACK_AREA2 5
#define MUSIC_TRACK_AREA3 6
#define MUSIC_TRACK_AREA4 7
#define MUSIC_TRACK_BOSS 8
#define MUSIC_TRACK_BOSS2 9
#define MUSIC_TRACK_END 10
#define MUSIC_TRACK_PICKUP 11
#define MUSIC_TRACK_FANFARE 12
#define MUSIC_TRACK_MAP_ITEM 13

#define REPEAT_NONE 0
#define REPEAT_LOOP 1
#define REPEAT_RESUME 2

void init_music();

void play_song(const unsigned char* song, char loop);

void tick_music();

void do_noise_effect(char note, char bend, char duration);

void stop_music();

void pause_music();

void unpause_music();

#endif