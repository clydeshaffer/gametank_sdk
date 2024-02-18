#include "music.h"
#include "gametank.h"
#include "dynawave.h"
#include "note_numbers.h"
#include "banking.h"

unsigned char channel_masks[NUM_FM_CHANNELS] = {1, 2, 4, 8};
signed char channel_note_offset[NUM_FM_CHANNELS] = {0, 0, 0, 0};
unsigned char audio_amplitudes[NUM_FM_OPS] = { 0, 0, 0, 0,
                                              0, 0, 0, 0,
                                              0, 0, 0, 0,
                                              0, 0, 0, 0 };
unsigned char env_initial[NUM_FM_OPS] = { 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00 };
unsigned char env_decay[NUM_FM_OPS] = { 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00 };
unsigned char env_sustain[NUM_FM_OPS] = { 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00 };

unsigned char op_transpose[NUM_FM_OPS] = { 0, 0, 0, 0, 
                                       28, 12, 0, 19,
                                       36, 0, 0, 0,
                                       0, 0, 0, 0 };


unsigned char* music_cursor = 0;
unsigned char delay_counter = 0;

unsigned char* repeat_point;
unsigned char* paused_cursor = 0;
unsigned char paused_delay;
unsigned char music_mode = REPEAT_NONE;
unsigned char repeat_resume_pending = 0;
unsigned char music_bank = 0;

void init_music() {
    music_cursor = 0;
    delay_counter = 0;
    stop_music();
}

void load_instrument(char channel, Instrument* instr) {
    channel_note_offset[channel] = instr->transpose;
    aram[FEEDBACK_AMT + channel] = instr->feedback + sine_offset;
    channel = channel << 2;
    env_initial[channel] = instr->env_initial[0];
    env_decay[channel] = instr->env_decay[0];
    env_sustain[channel] = instr->env_sustain[0];
    op_transpose[channel] = instr->op_transpose[0];
    ++channel;
    env_initial[channel] = instr->env_initial[1];
    env_decay[channel] = instr->env_decay[1];
    env_sustain[channel] = instr->env_sustain[1];
    op_transpose[channel] = instr->op_transpose[1];
    ++channel;
    env_initial[channel] = instr->env_initial[2];
    env_decay[channel] = instr->env_decay[2];
    env_sustain[channel] = instr->env_sustain[2];
    op_transpose[channel] = instr->op_transpose[2];
    ++channel;
    env_initial[channel] = instr->env_initial[3];
    env_decay[channel] = instr->env_decay[3];
    env_sustain[channel] = instr->env_sustain[3];
    op_transpose[channel] = instr->op_transpose[3];
}

void set_note(char ch, char n) {
    static char n_mul;
    n_mul = op_transpose[ch] + n;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul * 2]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul * 2 + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul * 2]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul * 2 + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul * 2]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul * 2 + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul * 2]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul * 2 + 1]);
}

void play_song(const unsigned char* song, char bank_num, char loop) {
    char *prev_cursor = music_cursor;
    music_bank = bank_num;
    change_rom_bank(music_bank);
    music_cursor = song;

    switch(loop) {
        case REPEAT_NONE:
            repeat_point = 0;
            music_mode = REPEAT_NONE;
            repeat_resume_pending = 0;
            break;
        case REPEAT_LOOP:
            repeat_point = music_cursor;
            music_mode = REPEAT_LOOP;
            repeat_resume_pending = 0;
            break;
        case REPEAT_RESUME:
            if(!repeat_resume_pending) {
                paused_cursor = prev_cursor;
                paused_delay = delay_counter;
                repeat_resume_pending = 1;
            }
            break;
    }

    if(music_cursor) {
        delay_counter = *(music_cursor++);
    }
}

void pause_music() {
    paused_cursor = music_cursor;
    paused_delay = delay_counter;
    music_cursor = 0;
}

void unpause_music() {
    music_cursor = paused_cursor;
    delay_counter = paused_delay;
    paused_cursor = 0;
}

void tick_music() {
    static unsigned char n, noteMask, a, op, ch;
    change_rom_bank(music_bank);
    for(op = 0; op < NUM_FM_OPS; ++op) {
        if(audio_amplitudes[op] > env_sustain[op]) {
            audio_amplitudes[op] -= env_decay[op];
            set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
        }
    }

    if(music_cursor) {
        loadAudioEvent:
        if(delay_counter > 0) {
            delay_counter--;
        } else {
            noteMask = *(music_cursor++);
            for(ch = 0; ch < NUM_FM_CHANNELS; ++ch) {
                if(noteMask & channel_masks[ch]) {
                    n = *(music_cursor++);
                    op = ch << 2;
                    if(n > 0) {
                        set_note(op, n + channel_note_offset[ch]);
                        audio_amplitudes[op] = env_initial[op];
                        set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        ++op;
                        audio_amplitudes[op] = env_initial[op];
                        set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        ++op;
                        audio_amplitudes[op] = env_initial[op];
                        set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        ++op;
                        audio_amplitudes[op] = env_initial[op];
                        set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                    } else {
                        audio_amplitudes[op] = 0;
                        set_audio_param(AMPLITUDE+op, sine_offset);
                        ++op;
                        audio_amplitudes[op] = 0;
                        set_audio_param(AMPLITUDE+op, sine_offset);
                        ++op;
                        audio_amplitudes[op] = 0;
                        set_audio_param(AMPLITUDE+op, sine_offset);
                        ++op;
                        audio_amplitudes[op] = 0;
                        set_audio_param(AMPLITUDE+op, sine_offset);
                    }
                }
            }
            delay_counter = *(music_cursor++);
            if(delay_counter == 0) {
                if(repeat_resume_pending != 0) {
                    repeat_resume_pending = 0;
                    music_cursor = paused_cursor;
                    delay_counter = paused_delay;
                    paused_cursor = 0;
                } else {
                    music_cursor = repeat_point;
                    if(music_cursor) {
                        delay_counter = *(music_cursor++);
                        goto loadAudioEvent;
                    }   
                }
            } else {
                --delay_counter;
            }
        }
    }


    flush_audio_params();
}

void do_noise_effect(char note, char bend, char duration) {
    set_note(2, note);
    //set_audio_param(PITCHBEND+2, bend);
    audio_amplitudes[2] = duration;
    set_audio_param(AMPLITUDE+2, 127);
    //flush_audio_params();
}

void stop_music() {
    char n;
    music_cursor = 0;
    for(n = 0; n < NUM_FM_OPS; ++n) {
        audio_amplitudes[n] = 0;
        set_audio_param(AMPLITUDE+n, sine_offset);
    }
    //flush_audio_params();
}