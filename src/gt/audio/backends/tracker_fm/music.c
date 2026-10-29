#include "../../../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_SDK_TRACKER_FM_BACKEND

#include "music.h"
#include "gametank.h"
#include "banking.h"

#include "../../../../gen/bank_nums.h"
#include "../../../../gen/assets/assets_index.h"
#include "../../../../gen/assets/sdk_default.h"
#include "../../../../gen/modules_enabled.h"

typedef struct music_state_t {
    track_header_t* song_base;
    unsigned char bank;
    unsigned char delay;
    unsigned char flags; //playback flags
    char instruments_idx[NUM_FM_CHANNELS];
    char channel_held_steps[NUM_FM_CHANNELS];
    char sequence_idx;
    char step_idx;
} music_state_t;

char* env_ptr_cache[64];
char channelStates[4];
char* channelSequences[4];
char sequenceLength;
char channelCurrentPattern[4];
char last_note[4];
#define MUSIC_STACK_SIZE 8

track_header_t track_header;
instrument_header_t* instruments[32];
music_state_t music_state;
music_state_t music_stack[MUSIC_STACK_SIZE];
unsigned char music_stack_idx = 0;

void push_song_stack() {
    music_stack[music_stack_idx++] = music_state;
    if(music_stack_idx == MUSIC_STACK_SIZE) { music_stack_idx = 0; }
}

void pop_song_stack() {
    char n;
    if(music_stack_idx == 0) {
        music_stack_idx = MUSIC_STACK_SIZE-1;
    } else { 
        --music_stack_idx;
    }
    music_state = music_stack[music_stack_idx];
    change_rom_bank(music_state.bank);

    track_header = *music_state.song_base;

    last_note[0] = 0;
    last_note[1] = 0;
    last_note[2] = 0;
    last_note[3] = 0;
    channelStates[0] = 255;
    channelStates[1] = 255;
    channelStates[2] = 255;
    channelStates[3] = 255;

    for(n = 0; n < NUM_FM_OPS; ++n) {
        set_audio_param(AMPLITUDE+n, 128);
    }
    init_song_data();
}

unsigned char channel_masks[NUM_FM_CHANNELS] = {1, 2, 4, 8};

unsigned char op_transpose[NUM_FM_OPS] = { 0, 0, 0, 0, 
                                       28, 12, 0, 19,
                                       36, 0, 0, 0,
                                       0, 0, 0, 0 };

#define MUSIC_CFG_VELOCITY 1
#define MUSIC_CFG_PROGCHANGE 2

#define MUSIC_FLAG_POP_AT_END 1
#define MUSIC_FLAG_REPEAT 2
#define MUSIC_FLAG_STOPPED 4

const unsigned char* paused_cursor = 0;
unsigned char paused_bank = 0;
unsigned char paused_cfg = 0;
unsigned char paused_delay;

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
    music_state.song_base = 0;
    music_state.delay = 0;
    music_channel_mask = 255;
    note_held_mask = 0;
    for(i = 0; i < NUM_FM_CHANNELS; ++i) {
        sound_effect_length[i] = 0;
        sound_effect_priority[i] = 0;
    }
    stop_music();
}

struct twoArgs {
    char a;
    char b;
};

char setEnvChannel, setEnvInst;
void setEnvelopeFetchers();
void fetchEnvelopes();

void load_instrument(char channel, char instrumentIdx) {
    music_state.instruments_idx[channel] = instrumentIdx;

    setEnvChannel = channel;
    setEnvInst = instrumentIdx;
    setEnvelopeFetchers();

    channel = channel << 2;
    op_transpose[channel] = instruments[instrumentIdx]->params[0];
    ++channel;
    op_transpose[channel] = instruments[instrumentIdx]->params[1];
    ++channel;
    op_transpose[channel] = instruments[instrumentIdx]->params[2];
    ++channel;
    op_transpose[channel] = instruments[instrumentIdx]->params[3];
}

char n_mul;
void set_note(char ch, char n) {
    ch = ch << 2;
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

void init_song_data() {
    char n;
    char* envelopePtrArray;
    for(n = 0; n < track_header.instrument_count; ++n) {
        instruments[n] = ((instrument_header_t*)(((char*) music_state.song_base->instruments[n]) + (int) music_state.song_base));
        envelopePtrArray = instruments[n]->envelopes;
        env_ptr_cache[n<<2]     = instruments[n]->envelopes[0] + ((int)music_state.song_base);
        env_ptr_cache[(n<<2)+1] = instruments[n]->envelopes[1] + ((int)music_state.song_base);
        env_ptr_cache[(n<<2)+2] = instruments[n]->envelopes[2] + ((int)music_state.song_base);
        env_ptr_cache[(n<<2)+3] = instruments[n]->envelopes[3] + ((int)music_state.song_base);
    }

    
    for(n = 0; n < NUM_FM_CHANNELS; ++n) {
        track_header.channel_datas[n] = (channel_header_t*)(((char*)(track_header.channel_datas[n])) + ((int) music_state.song_base));
        channelSequences[n] = track_header.channel_datas[n]->patternHeads[0] + ((int)music_state.song_base);
        channelCurrentPattern[n] = channelSequences[n][music_state.sequence_idx];
        load_instrument(n, music_state.instruments_idx[n]);
    }
    sequenceLength = track_header.channel_datas[0]->sequenceLength;
}

char evt;
char ins;

void play_song(const track_header_t* song, char bank_num, char loop) {
    char n;
    push_song_stack();
    music_state.bank = bank_num;
    push_rom_bank();
    change_rom_bank(music_state.bank);
    track_header = *song;
    music_state.song_base = song;
    music_state.sequence_idx = 0;
    music_state.delay = track_header.step_frames;
    music_state.step_idx = 0;
    last_note[0] = 0;
    last_note[1] = 0;
    last_note[2] = 0;
    last_note[3] = 0;
    channelStates[0] = 255;
    channelStates[1] = 255;
    channelStates[2] = 255;
    channelStates[3] = 255;

    for(n = 0; n < NUM_FM_OPS; ++n) {
        set_audio_param(AMPLITUDE+n, 128);
    }

    switch(loop) {
        case REPEAT_NONE: 
            music_state.flags = 0;
            break;
        case REPEAT_LOOP:
            music_state.flags = MUSIC_FLAG_REPEAT;
            break;
        case REPEAT_RESUME:
            music_state.flags = MUSIC_FLAG_POP_AT_END;
            break;
    }

    init_song_data();

    for(n = 0; n < NUM_FM_CHANNELS; ++n) {
        evt = (track_header.channel_datas[n]->patternHeads[channelCurrentPattern[n]+1] + ((int) music_state.song_base))[(music_state.step_idx) << 1];
        ins = (track_header.channel_datas[n]->patternHeads[channelCurrentPattern[n]+1] + ((int) music_state.song_base))[((music_state.step_idx) << 1)+1];
        if(evt != 255) {

            if(ins != music_state.instruments_idx[n]) {
                load_instrument(n, ins);
            }
            last_note[n] = evt;
            if(music_channel_mask & channel_masks[n]) {
                set_note(n, evt);
            }
            channelStates[n] = 1;
        } else {
            channelStates[n] = 255;
        }
    }
    fetchEnvelopes();

    pop_rom_bank();
}

void pause_music() {
    push_song_stack();
    note_held_mask = 0;
}

void unpause_music() {
    pop_song_stack();
}

void tick_music() {
    static unsigned char n;
    register unsigned char a, op;
    push_rom_bank();
    for(n = 0; n < NUM_FM_CHANNELS; ++n) {
        if(sound_effect_length[n]) {
            --sound_effect_length[n];
            if(sound_effect_length[n]) {
                
                change_rom_bank(sound_effect_bank[n]);

                op = n << 2;
                set_audio_param(AMPLITUDE + op, (*(sound_effect_ptr[n]+0) << 3) + 128);
                a = *(sound_effect_ptr[n]+4) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, (*(sound_effect_ptr[n]+1) << 3) + 128);
                a = *(sound_effect_ptr[n]+5) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, (*(sound_effect_ptr[n]+2) << 3) + 128);
                a = *(sound_effect_ptr[n]+6) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                ++op;
                set_audio_param(AMPLITUDE + op, (*(sound_effect_ptr[n]+3) << 3) + 128);
                a = *(sound_effect_ptr[n]+7) << 1;
                set_audio_param(PITCH_MSB + op, pitch_table[a]);
                set_audio_param(PITCH_LSB + op, pitch_table[a+1]);

                sound_effect_ptr[n] += 8;

            } else {
                op = n << 2;
                set_audio_param(AMPLITUDE+(op+3), 128);
                aram[FEEDBACK_AMT + n] = 128;//saved_feedback_value[n];
                music_channel_mask |= channel_masks[n];
                sound_effect_priority[n] = 0;
                if(channelStates[n] != 255) {
                    set_note(n, last_note[n]);
                }
            }
        }
    }

    change_rom_bank(music_state.bank);
    if(~music_state.flags & MUSIC_FLAG_STOPPED) {
        

        --music_state.delay;

        if(music_state.delay == 0) {
            music_state.delay = track_header.step_frames;
            ++music_state.step_idx;
            if(music_state.step_idx == track_header.pattern_length) {
                music_state.step_idx = 0;
                ++music_state.sequence_idx;
                if(music_state.sequence_idx == sequenceLength) {
                    if(music_state.flags & MUSIC_FLAG_REPEAT) {
                        music_state.sequence_idx = 0;
                    } else if(music_state.flags & MUSIC_FLAG_POP_AT_END) {
                        pop_song_stack();
                    } else {
                        music_state.flags |= MUSIC_FLAG_STOPPED;
                    }
                }
                for(n = 0; n < NUM_FM_CHANNELS; ++n) {
                    channelCurrentPattern[n] = channelSequences[n][music_state.sequence_idx];
                }
            }
            if((~music_state.flags) & MUSIC_FLAG_STOPPED) {
                for(n = 0; n < NUM_FM_CHANNELS; ++n) {
                    evt = (track_header.channel_datas[n]->patternHeads[channelCurrentPattern[n]+1] + ((int) music_state.song_base))[(music_state.step_idx) << 1];
                    ins = (track_header.channel_datas[n]->patternHeads[channelCurrentPattern[n]+1] + ((int) music_state.song_base))[((music_state.step_idx) << 1)+1];
                    if(evt != 255) {
                        if(ins != music_state.instruments_idx[n]) {
                            load_instrument(n, ins);
                        }
                        last_note[n] = evt;
                        if(music_channel_mask & channel_masks[n]) {
                            set_note(n, evt);
                        }
                        channelStates[n] = 0;
                    }
                }
            }
        }

        
    }

    for(n = 0; n < NUM_FM_CHANNELS; ++n) {
        if(channelStates[n] < 16) {
            ++channelStates[n];
        } else {
            //while(channelStates[n] == 16) {}
            channelStates[n] = 255;
        }
    }
    fetchEnvelopes();

    flush_audio_params();
    pop_rom_bank();
    
}

void silence_all_channels() {
    char n;
    for(n = 0; n < NUM_FM_OPS; ++n) {
        set_audio_param(AMPLITUDE+n, 128);
    }
    note_held_mask = 0;
}

void stop_music() {
    music_state.flags |= MUSIC_FLAG_STOPPED;
    silence_all_channels();
    channelStates[0] = 255;
    channelStates[1] = 255;
    channelStates[2] = 255;
    channelStates[3] = 255;
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

void play_boot_jingle() {
    play_song(ASSET__sdk_default__jingle_bin, REPEAT_NONE);
}
#endif

#ifdef ENABLE_MODULE_AUDIO_NONE
void tick_music() {}

void init_music() {}

void play_sound_effect(char sfx_id, char channel) {}

void stop_music() {}

void play_song(const unsigned char* song, char bank_num, char loop) {}

void play_boot_jingle() {}
#endif