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
#define OPS_PER_CHANNEL 4
#define NUM_FM_OPS 16

extern unsigned char audio_amplitudes[NUM_FM_OPS];

typedef struct {
    unsigned char env_initial[OPS_PER_CHANNEL];
    unsigned char env_decay[OPS_PER_CHANNEL];
    unsigned char env_sustain[OPS_PER_CHANNEL];
    unsigned char op_transpose[OPS_PER_CHANNEL];
    unsigned char feedback;
    signed char transpose;
} Instrument;

void load_instrument(char channel, Instrument* instr);

#endif