#include "music.h"
#include "gametank.h"
#include "dynawave.h"
#include "note_numbers.h"
#include "banking.h"

extern const unsigned char* MainMusic;
extern const unsigned char* SecondMusic;
extern const unsigned char* ThirdMusic;
extern const unsigned char* FourthMusic;
extern const unsigned char* TitleMusic;
extern const unsigned char* DiedMusic;
extern const unsigned char* StairsMusic;
extern const unsigned char* BossMusic;
extern const unsigned char* BossMusic2;
extern const unsigned char* EndMusic;
extern const unsigned char* PickupMusic;
extern const unsigned char* FanfareMusic;
extern const unsigned char* MapItemMusic;
#define NUM_FM_CHANNELS 4
#define NUM_FM_OPS 16
unsigned char channel_masks[NUM_FM_CHANNELS] = {1, 2, 4, 8};
signed char channel_note_offset[NUM_FM_CHANNELS] = {0, -12, -48, -12};
unsigned char audio_amplitudes[NUM_FM_OPS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char env_initial[NUM_FM_OPS] = { 0x30, 0x58, 0x78, 0x30, 
                                          0x40, 0x78, 0x7F, 0x70,
                                          0x20, 0x58, 0x7f, 0x7F,
                                          0x5F, 0x5F, 0x4F, 0x5F };
unsigned char env_decay[NUM_FM_OPS] = { 0x04, 0x18, 0x18, 0x01,
                                        0x02, 0x08, 0x00, 0x01,
                                        0x10, 0x04, 0x00, 0x01,
                                        0x02, 0x02, 0x04, 0x02 };
unsigned char env_sustain[NUM_FM_OPS] = { 0x04, 0x18, 0x18, 0x01,
                                        0x02, 0x08, 0x08, 0x01,
                                        0x10, 0x04, 0x08, 0x1,
                                        0x02, 0x02, 0x08, 0x40 };

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
    static unsigned char n, noteMask, a, op;
    change_rom_bank(music_bank);
    for(op = 0; op < NUM_FM_OPS; ++op) {
        if(audio_amplitudes[op] > env_sustain[op]) {
            audio_amplitudes[op] -= env_decay[op];
            push_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
        }
    }

    if(music_cursor) {
        if(delay_counter > 0) {
            delay_counter--;
        } else {
            noteMask = *(music_cursor++);
            for(op = 0; op < NUM_FM_CHANNELS; ++op) {
                if(noteMask & channel_masks[op]) {
                    n = *(music_cursor++);
                    if(n > 0) {
                        set_note(op, n + channel_note_offset[op]);
                        audio_amplitudes[op] = env_initial[op];
                        push_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        op += 4;
                        audio_amplitudes[op] = env_initial[op];
                        push_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        op += 4;
                        audio_amplitudes[op] = env_initial[op];
                        push_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        op += 4;
                        audio_amplitudes[op] = env_initial[op];
                        push_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        op -= 12;

                    } else {
                        audio_amplitudes[op] = 0;
                        push_audio_param(AMPLITUDE+op, sine_offset);
                        op += 4;
                        audio_amplitudes[op] = 0;
                        push_audio_param(AMPLITUDE+op, sine_offset);
                        op += 4;
                        audio_amplitudes[op] = 0;
                        push_audio_param(AMPLITUDE+op, sine_offset);
                        op += 4;
                        audio_amplitudes[op] = 0;
                        push_audio_param(AMPLITUDE+op, sine_offset);
                        op -= 12;
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
                    }   
                }
            }
        }
    }


    flush_audio_params();
}

void do_noise_effect(char note, char bend, char duration) {
    set_note(2, note);
    push_audio_param(PITCHBEND+2, bend);
    audio_amplitudes[2] = duration;
    push_audio_param(AMPLITUDE+2, 127);
    flush_audio_params();
}

void stop_music() {
    music_cursor = 0;
    audio_amplitudes[0] = 0;
    audio_amplitudes[1] = 0;
    audio_amplitudes[2] = 0;
    audio_amplitudes[3] = 0;
    push_audio_param(AMPLITUDE+0, 0);
    push_audio_param(AMPLITUDE+1, 0);
    push_audio_param(AMPLITUDE+2, 0);
    push_audio_param(AMPLITUDE+3, 0);
    push_audio_param(AMPLITUDE+4, 0);
    push_audio_param(AMPLITUDE+5, 0);
    push_audio_param(AMPLITUDE+6, 0);
    push_audio_param(AMPLITUDE+7, 0);
    push_audio_param(AMPLITUDE+8, 0);
    push_audio_param(AMPLITUDE+9, 0);
    push_audio_param(AMPLITUDE+10, 0);
    push_audio_param(AMPLITUDE+11, 0);
    push_audio_param(AMPLITUDE+12, 0);
    push_audio_param(AMPLITUDE+13, 0);
    push_audio_param(AMPLITUDE+14, 0);
    push_audio_param(AMPLITUDE+15, 0);
    flush_audio_params();
}