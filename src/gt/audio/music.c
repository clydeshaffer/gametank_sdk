#include "music.h"
#include "gametank.h"
#include "note_numbers.h"
#include "banking.h"

#include "../../gen/bank_nums.h"
#include "../../gen/assets/assets_index.h"

typedef struct music_state_t {
    const unsigned char* cursor;
    unsigned char bank;
    unsigned char cfg; //file flags
    unsigned char delay;
    const unsigned char* repeat_ptr;
    unsigned char flags; //playback flags
    const Instrument* instruments[NUM_FM_CHANNELS];
} music_state_t;

#define MUSIC_STACK_SIZE 8

music_state_t music_state;
music_state_t music_stack[MUSIC_STACK_SIZE];
unsigned char music_stack_idx = 0;

void push_song_stack() {
    music_stack[music_stack_idx++] = music_state;
    if(music_stack_idx == MUSIC_STACK_SIZE) { music_stack_idx = 0; }
}

void pop_song_stack() { 
    if(music_stack_idx == 0) {
        music_stack_idx = MUSIC_STACK_SIZE-1;
    } else { 
        --music_stack_idx;
    }
    music_state = music_stack[music_stack_idx];
    load_instrument(0, music_state.instruments[0]);
    load_instrument(0, music_state.instruments[1]);
    load_instrument(0, music_state.instruments[2]);
    load_instrument(0, music_state.instruments[3]);
}

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

#define MUSIC_CFG_VELOCITY 1
#define MUSIC_CFG_PROGCHANGE 2

#define MUSIC_FLAG_POP_AT_END 1

const unsigned char* paused_cursor = 0;
unsigned char paused_bank = 0;
unsigned char paused_cfg = 0;
unsigned char paused_delay;
unsigned char music_mode = REPEAT_NONE;

unsigned char music_channel_mask;
unsigned char percussion_channel_mask;
unsigned char note_held_mask;
const unsigned char* sound_effect_ptr[NUM_FM_CHANNELS];
unsigned char sound_effect_bank[NUM_FM_CHANNELS];
unsigned char sound_effect_length[NUM_FM_CHANNELS];
unsigned char saved_feedback_value[NUM_FM_CHANNELS];
unsigned char sound_effect_priority[NUM_FM_CHANNELS];

void init_music() {
    char i;
    music_stack_idx = 0;
    music_state.cursor = 0;
    music_state.delay = 0;
    music_channel_mask = 255;
    note_held_mask = 0;
    for(i = 0; i < NUM_FM_CHANNELS; ++i) {
        sound_effect_length[i] = 0;
        sound_effect_priority[i] = 0;
    }
    stop_music();
}

void load_instrument(char channel, const Instrument* instr) {
    if(instr == ((Instrument*) 0xFFFF)) {
        percussion_channel_mask |= channel_masks[channel];
        return;
    }
    percussion_channel_mask &= ~channel_masks[channel];
    music_state.instruments[channel] = instr;
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

char n_mul;
void set_note(char ch, char n) {
    n_mul = op_transpose[ch] + n;
    n_mul += n_mul;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    n_mul += n_mul;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    n_mul += n_mul;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul + 1]);
    ++ch;
    n_mul = op_transpose[ch] + n;
    n_mul += n_mul;
    set_audio_param(PITCH_MSB + ch, pitch_table[n_mul]);
    set_audio_param(PITCH_LSB + ch, pitch_table[n_mul + 1]);
}

void play_song(const unsigned char* song, char bank_num, char loop) {
    char n;
    push_song_stack();
    music_state.bank = bank_num;
    push_rom_bank();
    change_rom_bank(music_state.bank);
    music_state.cursor = song;

    for(n = 0; n < NUM_FM_OPS; ++n) {
        audio_amplitudes[n] = 0;
        set_audio_param(AMPLITUDE+n, sine_offset);
    }

    switch(loop) {
        case REPEAT_NONE: 
            music_state.repeat_ptr = 0;
            music_mode = REPEAT_NONE;
            music_state.flags = 0;
            break;
        case REPEAT_LOOP:
            music_state.repeat_ptr = music_state.cursor;
            music_mode = REPEAT_LOOP;
            music_state.flags = 0;
            break;
        case REPEAT_RESUME:
            music_state.flags = MUSIC_FLAG_POP_AT_END;
            break;
    }

    if(music_state.cursor) {
        percussion_channel_mask = 0;
        music_state.cfg = *(music_state.cursor++);
        load_instrument(0, get_instrument_ptr(*(music_state.cursor++)));
        load_instrument(1, get_instrument_ptr(*(music_state.cursor++)));
        load_instrument(2, get_instrument_ptr(*(music_state.cursor++)));
        load_instrument(3, get_instrument_ptr(*(music_state.cursor++)));
        music_state.delay = *(music_state.cursor++);
    }

    pop_rom_bank();
}

void pause_music() {
    push_song_stack();
    music_state.cursor = 0;
    note_held_mask = 0;
}

void unpause_music() {
    pop_song_stack();
}

void tick_music() {
    static unsigned char n, noteMask, ch;
    register unsigned char a, op;
    push_rom_bank();
    for(n = 0; n < NUM_FM_CHANNELS; ++n) {
        if(sound_effect_length[n]) {
            --sound_effect_length[n];
            if(sound_effect_length[n]) {
                
                change_rom_bank(sound_effect_bank[n]);

                op = n << 2;
                set_audio_param(AMPLITUDE + op, *(sound_effect_ptr[n]+0) + sine_offset);
                a = *(sound_effect_ptr[n]+4) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, *(sound_effect_ptr[n]+1) + sine_offset);
                a = *(sound_effect_ptr[n]+5) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, *(sound_effect_ptr[n]+2) + sine_offset);
                a = *(sound_effect_ptr[n]+6) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, *(sound_effect_ptr[n]+3) + sine_offset);
                a = *(sound_effect_ptr[n]+7) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                sound_effect_ptr[n] += 8;

            } else {
                op = n << 2;
                set_audio_param(AMPLITUDE+(op+3), sine_offset);
                aram[FEEDBACK_AMT + n] = saved_feedback_value[n];
                music_channel_mask |= channel_masks[n];
                sound_effect_priority[n] = 0;
            }
        }
    }

    change_rom_bank(music_state.bank);
    op = 0;
    for(ch = 1; ch < 16; ch = ch << 1) {
        if(ch & ~(music_channel_mask & note_held_mask)) {
            op+=4;
            continue;
        }
        for(a = 0; a < 4; ++a) {
            if(((env_sustain[op] - audio_amplitudes[op]) ^ (env_decay[op])) & 0x80) {
                audio_amplitudes[op] -= env_decay[op];
            } else {
                audio_amplitudes[op] = env_sustain[op];
            }
            set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
            ++op;
        }
    }

    if(music_state.cursor) {
        loadAudioEvent:
        if(music_state.delay > 0) {
            music_state.delay--;
        } else {
            noteMask = *(music_state.cursor++);
            for(ch = 0; ch < NUM_FM_CHANNELS; ++ch) {
                if(channel_masks[ch] & noteMask) {
                    n = *(music_state.cursor++);
                    if(music_state.cfg & MUSIC_CFG_VELOCITY)
                        a = *(music_state.cursor++);
                    if(channel_masks[ch] & percussion_channel_mask) {
                        if(n > 0) play_sound_effect(n - 1, ch);
                    } else if(channel_masks[ch] & music_channel_mask) {
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
                            if(music_state.cfg & MUSIC_CFG_VELOCITY)
                                audio_amplitudes[op] = a;
                            else
                                audio_amplitudes[op] = env_initial[op];
                            set_audio_param(AMPLITUDE+op, (audio_amplitudes[op] >> 4) + sine_offset);
                        } else {
                            audio_amplitudes[op+3] = 0;
                            set_audio_param(AMPLITUDE+op+3, sine_offset);
                        }
                    }
                    if(n) {
                        note_held_mask |= channel_masks[ch];
                    } else {
                        note_held_mask &= ~channel_masks[ch];
                    }
                }
            }
            music_state.delay = *(music_state.cursor++);
            if(music_state.delay == 0) {
                if(music_state.flags & MUSIC_FLAG_POP_AT_END) {
                    pop_song_stack();
                } else {
                    music_state.cursor = music_state.repeat_ptr;
                    if(music_state.cursor) {
                        music_state.cursor+=5; //skip cfg and instruments
                        music_state.delay = *(music_state.cursor++);
                        goto loadAudioEvent;
                    } else {
                        silence_all_channels();
                    }
                }
            } else {
                --music_state.delay;
            }
        }
    }

    flush_audio_params();
    pop_rom_bank();
}

void silence_all_channels() {
    char n;
    for(n = 0; n < NUM_FM_OPS; ++n) {
        audio_amplitudes[n] = 0;
        set_audio_param(AMPLITUDE+n, sine_offset);
    }
    note_held_mask = 0;
}

void stop_music() {
    music_state.cursor = 0;
    silence_all_channels();
}

void play_sound_effect(char sfx_id, char channel) { 
    static char priority;
    priority = channel & 0xF0;
    channel &= 0x03;
    if(priority < sound_effect_priority[channel]) return;
    sound_effect_priority[channel] = priority;
    push_rom_bank();
    change_rom_bank(BANK_LOADERS);
    sound_effect_bank[channel] = ASSET__sfx_bank_table[sfx_id];
    sound_effect_ptr[channel] = ASSET__sfx_ptr_table[sfx_id];
    change_rom_bank(sound_effect_bank[channel]);
    sound_effect_length[channel] = *(sound_effect_ptr[channel]++) + 1;
    saved_feedback_value[channel] = aram[FEEDBACK_AMT + channel];
    aram[FEEDBACK_AMT + channel] = *(sound_effect_ptr[channel]++);
    music_channel_mask &= ~(channel_masks[channel]);
    pop_rom_bank();
}
