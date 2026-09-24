#include "../../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_GTTAUDIO

#include <zlib.h>
#include "wavetable_audio.h"
#include "gametank.h"
#include "banking.h"
#include "../../../gen/bank_nums.h"

extern const unsigned char WavetableFWPkg;

const unsigned int WAVETABLE[WAVETABLE_COUNT] = {
    0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00,
    NOISE_SENTINEL_MODE0
};

void init_wavetable_audio() {
    *audio_rate = 0;

    push_rom_bank();
    change_rom_bank(BANK_COMMON);
    inflatemem(aram, &WavetableFWPkg);
    pop_rom_bank();

    *audio_reset = 1;
    *audio_rate = WAVETABLE_SAMPLE_RATE_REG;
}

void load_wavetable_instrument(char slot, const unsigned char *table) {
    unsigned int i;
    unsigned char *dest = aram + WAVETABLE[(unsigned char) slot];
    for (i = 0; i < WAVETABLE_SIZE; i++) {
        dest[i] = table[i];
    }
}

void mute_voice(char voice) {
    VOICES[(unsigned char) voice].volume = 0;
}

void mute_all_voices() {
    char i;
    for (i = 0; i < VOICE_COUNT; i++) {
        mute_voice(i);
    }
}

#endif
