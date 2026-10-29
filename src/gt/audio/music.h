#include "../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_SDK_TRACKER_FM_BACKEND
#include "backends/tracker_fm/music.h"
#endif

#ifdef ENABLE_MODULE_LEGACY_MIDI_FM_BACKEND
#include "backends/legacy_midi_fm/music.h"
#endif

#ifdef ENABLE_MODULE_AUDIO_NONE
void tick_music();

void init_music();

void play_sound_effect(char sfx_id, char channel);

void stop_music();

void play_song(const unsigned char* song, char bank_num, char loop);

void play_boot_jingle();
#endif