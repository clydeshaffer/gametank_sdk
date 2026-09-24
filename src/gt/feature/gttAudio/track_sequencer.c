#include "../../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_GTTAUDIO

#include "track_sequencer.h"
#include "wavetable_audio.h"
#include "gametank.h"
#include "banking.h"
#include "../../../gen/bank_nums.h"
#include "../../../gen/assets/instruments.h"

/* Number of parallel per-beat arrays packed into each channel's slice of a
 * pattern's data block:
 * - freq_lo
 * - freq_hi
 * - vol
 * - fx_id
 * - fx_x
 * - fx_y
 * - fx_freq_x_lo
 * - fx_freq_x_hi
 * - fx_freq_y_lo
 * - fx_freq_y_hi
 */
#define CHANNEL_ARRAYS 10

#define FX_ID_INSTRUMENT 1
#define FX_ID_ARPEGGIO 2
#define FX_ID_PITCH_UP 3
#define FX_ID_PITCH_DOWN 4
#define FX_ID_FADE_IN 5
#define FX_ID_FADE_OUT 6
#define FX_ID_TREMBLE 7
#define ARP_NO_THIRD_NOTE 0xFF

/* See the gt-tracker README for the track descriptor layout */
#define TRACK_BPM_OFFSET 0
#define TRACK_SPEED_OFFSET 2
#define TRACK_PATTERN_COUNT_OFFSET 4
#define TRACK_HEADER_SIZE 5

/* seq_cmd_type values baked into each pattern's trailing per-beat arrays */
#define SEQ_CMD_STOP 1
#define SEQ_CMD_TEMPO 2
#define SEQ_CMD_SPEED 3
#define SEQ_CMD_FLOW_COUNT 4
#define SEQ_CMD_COUNT_JUMP 5
#define SEQ_CMD_JUMP 6

/* Channel base note pitch, written by advance_beat. This is different than
 * the active note used by advance_tick which represents the frequency after
 * applied effects. */
static unsigned int base_freq[VOICE_COUNT];
/* Which glide effect is currently active */
#define ACTIVE_FX_NONE 0
#define ACTIVE_FX_ARP 1
#define ACTIVE_FX_PITCH 2
#define ACTIVE_FX_FADE 3
#define ACTIVE_FX_TREMBLE 4
static unsigned char active_fx[VOICE_COUNT];
static unsigned int arp_x_freq[VOICE_COUNT];
static unsigned int arp_y_freq[VOICE_COUNT];
/* Keep track if the channel has a 2-note or 3-note arpeggio */
static unsigned char arp_step_count[VOICE_COUNT] = {3, 3, 3, 3, 3, 3, 3};
/* Each channel's current position within its own arpeggio cycle */
static unsigned char arp_step[VOICE_COUNT];
/* The baked frequency the glide is sliding toward */
static unsigned int pitch_target[VOICE_COUNT];
/* Current frequency in the pitch slide */
static unsigned int pitch_cur[VOICE_COUNT];
/* VOL reference so FadeIn/FadeOut don't need to read hardware state back. */
static unsigned char cur_vol[VOICE_COUNT];
/* Volume the fade is sliding toward (cur_vol or 0) */
static unsigned char fade_target[VOICE_COUNT];
/* The current volume of the fade */
static unsigned char fade_cur[VOICE_COUNT];
/* Baked `x` param: ticks to hold each Fade volume step, or each Tremble on/off phase. */
static unsigned char vol_fx_hold[VOICE_COUNT];
/* Ticks remaining until the next Fade step or Tremble toggle */
static unsigned char vol_fx_counter[VOICE_COUNT];
/* Whether the channel is currently in the muted half of its tremble cycle */
static unsigned char tremble_muted[VOICE_COUNT];

static unsigned int read_u16(const unsigned char *base, unsigned int offset) {
    return *((const unsigned int *) (base + offset));
}

static const unsigned char *pattern_ptr(TrackSequencer *seq, unsigned char pattern_idx) {
    const unsigned int *table = (const unsigned int *) (seq->track + TRACK_HEADER_SIZE);
    unsigned int pattern_offset = table[pattern_idx];
    return seq->track + pattern_offset;
}

static void advance_tick(TrackSequencer *seq) {
    unsigned char ch;
    unsigned int freq;
    unsigned int speed = seq->speed << 2;
    for (ch = 0; ch < VOICE_COUNT; ch++) {
        if (active_fx[ch] == ACTIVE_FX_ARP) {
            switch (arp_step[ch]) {
                case 0: freq = base_freq[ch]; break;
                case 1: freq = arp_x_freq[ch]; break;
                default: freq = arp_y_freq[ch]; break;
            }
            VOICES[ch].frequency = freq;
            arp_step[ch] = (arp_step[ch] + 1 >= arp_step_count[ch]) ? 0 : arp_step[ch] + 1;
        } else if (active_fx[ch] == ACTIVE_FX_PITCH) {
            unsigned int cur = pitch_cur[ch];
            unsigned int target = pitch_target[ch];
            unsigned int next;
            if (cur < target) {
                unsigned int stepped = cur + speed;
                if (stepped >= target) {
                    active_fx[ch] = ACTIVE_FX_NONE;
                    next = target;
                } else {
                    next = stepped;
                }
            } else if (cur > target) {
                unsigned int stepped = (cur > speed) ? cur - speed : 0;
                if (stepped <= target) {
                    active_fx[ch] = ACTIVE_FX_NONE;
                    next = target;
                } else {
                    next = stepped;
                }
            } else {
                active_fx[ch] = ACTIVE_FX_NONE;
                next = target;
            }
            pitch_cur[ch] = next;
            VOICES[ch].frequency = next;
        } else if (active_fx[ch] == ACTIVE_FX_FADE) {
            if (vol_fx_counter[ch] == 0) {
                unsigned char cur = fade_cur[ch];
                unsigned char target = fade_target[ch];
                unsigned char next_vol;
                if (cur < target) {
                    next_vol = cur + 1;
                } else if (cur > target) {
                    next_vol = cur - 1;
                } else {
                    next_vol = cur;
                }
                fade_cur[ch] = next_vol;
                VOICES[ch].volume = next_vol;
                if (next_vol == target) {
                    active_fx[ch] = ACTIVE_FX_NONE;
                }
                vol_fx_counter[ch] = vol_fx_hold[ch];
            } else {
                vol_fx_counter[ch]--;
            }
        } else if (active_fx[ch] == ACTIVE_FX_TREMBLE) {
            if (vol_fx_counter[ch] == 0) {
                tremble_muted[ch] = !tremble_muted[ch];
                VOICES[ch].volume = tremble_muted[ch] ? 0 : cur_vol[ch];
                vol_fx_counter[ch] = vol_fx_hold[ch];
            } else {
                vol_fx_counter[ch]--;
            }
        }
    }
}

static unsigned char trigger_channels(const unsigned char *data, unsigned int channel_stride, unsigned int pattern_beats, unsigned int beat) {
    unsigned int off_freq_lo = 0;
    unsigned int off_freq_hi = off_freq_lo + pattern_beats;
    unsigned int off_vol = off_freq_hi + pattern_beats;
    unsigned int off_fx_id = off_vol + pattern_beats;
    unsigned int off_fx_x = off_fx_id + pattern_beats;
    unsigned int off_fx_y = off_fx_x + pattern_beats;
    unsigned int off_fx_freq_x_lo = off_fx_y + pattern_beats;
    unsigned int off_fx_freq_x_hi = off_fx_freq_x_lo + pattern_beats;
    unsigned int off_fx_freq_y_lo = off_fx_freq_x_hi + pattern_beats;
    unsigned int off_fx_freq_y_hi = off_fx_freq_y_lo + pattern_beats;

    unsigned int base = 0;
    unsigned char any_active = 0;
    unsigned char ch;

    for (ch = 0; ch < VOICE_COUNT; ch++) {
        unsigned int lo = data[base + off_freq_lo + beat];
        unsigned int hi = data[base + off_freq_hi + beat];
        unsigned char vol = data[base + off_vol + beat];
        unsigned char fx_id = data[base + off_fx_id + beat];
        unsigned char fx_x = data[base + off_fx_x + beat];
        unsigned char fx_y = data[base + off_fx_y + beat];
        unsigned char prev_fx = active_fx[ch];
        unsigned char had_freq_fx = (prev_fx == ACTIVE_FX_ARP || prev_fx == ACTIVE_FX_PITCH);
        unsigned char had_vol_fx = (prev_fx == ACTIVE_FX_FADE || prev_fx == ACTIVE_FX_TREMBLE);

        unsigned char new_note = (lo | hi) != 0;
        if (new_note) {
            base_freq[ch] = lo | (hi << 8);
        }
        if (vol != 0xFF) {
            cur_vol[ch] = vol;
            VOICES[ch].volume = vol;
        }

        if (fx_id == FX_ID_ARPEGGIO) {
            unsigned int arp_x_lo = data[base + off_fx_freq_x_lo + beat];
            unsigned int arp_x_hi = data[base + off_fx_freq_x_hi + beat];
            unsigned int arp_y_lo = data[base + off_fx_freq_y_lo + beat];
            unsigned int arp_y_hi = data[base + off_fx_freq_y_hi + beat];
            active_fx[ch] = ACTIVE_FX_ARP;
            arp_x_freq[ch] = arp_x_lo | (arp_x_hi << 8);
            arp_y_freq[ch] = arp_y_lo | (arp_y_hi << 8);
            arp_step_count[ch] = (fx_y == ARP_NO_THIRD_NOTE) ? 2 : 3;
            arp_step[ch] = 0;
            any_active = 1;
        } else if (fx_id == FX_ID_PITCH_UP || fx_id == FX_ID_PITCH_DOWN) {
            unsigned int target_lo = data[base + off_fx_freq_x_lo + beat];
            unsigned int target_hi = data[base + off_fx_freq_x_hi + beat];
            active_fx[ch] = ACTIVE_FX_PITCH;
            pitch_target[ch] = target_lo | (target_hi << 8);
            pitch_cur[ch] = base_freq[ch];
            VOICES[ch].frequency = pitch_cur[ch];
            any_active = 1;
        } else if (fx_id == FX_ID_FADE_IN || fx_id == FX_ID_FADE_OUT) {
            active_fx[ch] = ACTIVE_FX_FADE;
            if (fx_id == FX_ID_FADE_IN) {
                fade_target[ch] = cur_vol[ch];
                fade_cur[ch] = 0;
            } else {
                fade_target[ch] = 0;
                fade_cur[ch] = cur_vol[ch];
            }
            VOICES[ch].volume = fade_cur[ch];
            vol_fx_hold[ch] = fx_x;
            vol_fx_counter[ch] = fx_x;
            if (new_note || had_freq_fx) {
                VOICES[ch].frequency = base_freq[ch];
            }
            any_active = 1;
        } else if (fx_id == FX_ID_INSTRUMENT) {
            VOICES[ch].wavetable = (fx_x == NOISE_INSTRUMENT)
                ? (fx_y == 0 ? NOISE_SENTINEL_MODE0 : NOISE_SENTINEL_MODE1)
                : WAVETABLE[fx_x];
            active_fx[ch] = ACTIVE_FX_NONE;
            if (new_note || had_freq_fx) {
                VOICES[ch].frequency = base_freq[ch];
            }
            if (had_vol_fx) {
                VOICES[ch].volume = cur_vol[ch];
            }
        } else if (fx_id == FX_ID_TREMBLE) {
            active_fx[ch] = ACTIVE_FX_TREMBLE;
            vol_fx_hold[ch] = fx_x;
            vol_fx_counter[ch] = fx_x;
            tremble_muted[ch] = 0;
            if (new_note || had_freq_fx) {
                VOICES[ch].frequency = base_freq[ch];
            }
            any_active = 1;
        } else {
            active_fx[ch] = ACTIVE_FX_NONE;
            if (new_note || had_freq_fx) {
                VOICES[ch].frequency = base_freq[ch];
            }
            if (had_vol_fx) {
                VOICES[ch].volume = cur_vol[ch];
            }
        }
        base += channel_stride;
    }
    return any_active;
}

static void advance_beat(TrackSequencer *seq) {
    for (;;) {
        const unsigned char *pat = pattern_ptr(seq, seq->pattern_idx);
        unsigned int pattern_beats = *pat;
        const unsigned char *data = pat + 1;
        unsigned int channel_stride = pattern_beats * CHANNEL_ARRAYS;
        unsigned int beat = seq->beat;
        unsigned int seq_cmd_base = channel_stride * VOICE_COUNT;
        unsigned int off_seq_cmd_type = 0;
        unsigned int off_seq_cmd_value = off_seq_cmd_type + pattern_beats;
        unsigned int off_seq_cmd_value2 = off_seq_cmd_value + pattern_beats;
        unsigned char seq_cmd_type = data[seq_cmd_base + off_seq_cmd_type + beat];
        unsigned char seq_cmd_value = data[seq_cmd_base + off_seq_cmd_value + beat];
        unsigned char seq_cmd_value2 = data[seq_cmd_base + off_seq_cmd_value2 + beat];
        unsigned char max_idx;

        switch (seq_cmd_type) {
            case SEQ_CMD_STOP:
                seq->stopped = 1;
                return;
            case SEQ_CMD_TEMPO:
                seq->bpm = seq_cmd_value;
                break;
            case SEQ_CMD_SPEED:
                seq->speed = seq_cmd_value;
                break;
            case SEQ_CMD_FLOW_COUNT:
                seq->flow_count = seq_cmd_value;
                break;
            case SEQ_CMD_COUNT_JUMP:
                if (seq->flow_count > 0) {
                    seq->flow_count--;
                    max_idx = (seq->pattern_count > 0) ? seq->pattern_count - 1 : 0;
                    seq->pattern_idx = (seq_cmd_value < max_idx) ? seq_cmd_value : max_idx;
                    seq->beat = seq_cmd_value2;
                    continue;
                }
                break;
            case SEQ_CMD_JUMP:
                max_idx = (seq->pattern_count > 0) ? seq->pattern_count - 1 : 0;
                seq->pattern_idx = (seq_cmd_value < max_idx) ? seq_cmd_value : max_idx;
                seq->beat = seq_cmd_value2;
                continue;
            default:
                break;
        }

        seq->fx_active_any = trigger_channels(data, channel_stride, pattern_beats, beat);

        seq->beat++;
        if (seq->beat >= pattern_beats) {
            seq->beat = 0;
        }
        return;
    }
}

void track_sequencer_init(TrackSequencer *seq, const unsigned char *track, unsigned char track_bank) {
    seq->track = track;
    seq->track_bank = track_bank;
    seq->beat = 0;
    seq->frame_acc = 0;
    seq->tick_acc = 0;
    seq->pattern_idx = 0;
    seq->flow_count = 0;
    seq->stopped = 0;
    seq->fx_active_any = 0;

    push_rom_bank();
    change_rom_bank(track_bank);
    seq->bpm = read_u16(track, TRACK_BPM_OFFSET);
    seq->speed = read_u16(track, TRACK_SPEED_OFFSET);
    seq->pattern_count = track[TRACK_PATTERN_COUNT_OFFSET];
    pop_rom_bank();
}

void track_sequencer_init_voices() {
    unsigned char i;

    push_rom_bank();
    change_rom_bank(BANK_instruments);
    load_wavetable_instrument(0, ASSET__instruments__instrument_0_raw_ptr);
    load_wavetable_instrument(1, ASSET__instruments__instrument_1_raw_ptr);
    load_wavetable_instrument(2, ASSET__instruments__instrument_2_raw_ptr);
    load_wavetable_instrument(3, ASSET__instruments__instrument_3_raw_ptr);
    load_wavetable_instrument(4, ASSET__instruments__instrument_4_raw_ptr);
    load_wavetable_instrument(5, ASSET__instruments__instrument_5_raw_ptr);
    load_wavetable_instrument(6, ASSET__instruments__instrument_6_raw_ptr);
    load_wavetable_instrument(7, ASSET__instruments__instrument_7_raw_ptr);
    load_wavetable_instrument(8, ASSET__instruments__instrument_8_raw_ptr);
    load_wavetable_instrument(9, ASSET__instruments__instrument_9_raw_ptr);
    pop_rom_bank();

    for (i = 0; i < VOICE_COUNT; i++) {
        VOICES[i].wavetable = WAVETABLE[i];
        VOICES[i].volume = 0;
    }
    *audio_rate = WAVETABLE_SAMPLE_RATE_REG;
}

void track_sequencer_tick(TrackSequencer *seq) {
    unsigned int remaining;

    if (seq->stopped) {
        return;
    }

    seq->frame_acc += seq->bpm;

    if (seq->fx_active_any) {
        remaining = seq->speed;
        while (remaining > 0) {
            seq->tick_acc += seq->bpm;
            remaining--;
        }

        while (seq->tick_acc >= 1800) {
            seq->tick_acc -= 1800;
            advance_tick(seq);
        }
    }

    if (seq->frame_acc >= 1800) {
        seq->frame_acc -= 1800;
        push_rom_bank();
        change_rom_bank(seq->track_bank);
        advance_beat(seq);
        pop_rom_bank();
    }
}

#endif
