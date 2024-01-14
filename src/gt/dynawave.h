#ifndef DYNAWAVE_H

#define DYNAWAVE_H

#define PITCH_MSB 0x10

#define PITCH_LSB 0x20

#define AMPLITUDE 0x30

#define AUDIO_PARAM_INPUT_BUFFER ((volatile char *) 0x3070)
#define WAVE_TABLE_LOCATION ((volatile unsigned int *) 0x3002)

extern char pitch_table[216];

void init_dynawave();

void push_audio_param(char param, char value);
void set_audio_param(char param, char value);

void flush_audio_params();

extern char* wavetable_page;

extern char sine_offset;

#endif