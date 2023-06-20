#ifndef DYNAWAVE_H

#define DYNAWAVE_H

#define PITCH_MSB 0x10

#define PITCH_LSB 0x14

#define AMPLITUDE 0x18

#define PITCHBEND 0x1C

#define AUDIO_PARAM_INPUT_BUFFER ((volatile char *) 0x3070)
#define WAVE_TABLE_LOCATION ((volatile int *) 0x3030)

void init_dynawave();

void push_audio_param(char param, char value);

void flush_audio_params();

void set_note(char ch, char n);

extern char* wavetable_page;

#endif