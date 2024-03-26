#include "gt/gametank.h"
#include "gt/drawing_funcs.h"
#include "gt/dynawave.h"
#include "gt/music.h"
#include "gen/assets/music.h"
#include "gen/assets/music2.h"
#include "gen/assets/music3.h"
#include "input.h";

char song_number = 0;

#define MAX_SONG_ID 11

void play_track(char num) {
  stop_music();
  switch(num) {
    case 1:
      play_song(&ASSET__music2__brinstar_mid, REPEAT_LOOP);
      return;
    case 2:
      play_song(&ASSET__music2__vampire_mid, REPEAT_LOOP);
      return;
    case 3:
      play_song(&ASSET__music__stroll_mid, REPEAT_LOOP);
      return;
    case 4:
      play_song(&ASSET__music__cubeknight_mid, REPEAT_LOOP);
      return;
    case 5:
      play_song(&ASSET__music__fiend_loop_mid, REPEAT_LOOP);
      return;
    case 6:
      play_song(&ASSET__music__darker_loop_mid, REPEAT_LOOP);
      return;
    case 7:
      play_song(&ASSET__music__march_loop_mid, REPEAT_LOOP);
      return;
    case 8:
      play_song(&ASSET__music__spooky_loop_mid, REPEAT_LOOP);
      return;
    case 9:
      play_song(&ASSET__music__megalo1_mid, REPEAT_LOOP);
      return;
    case 10:
      play_song(&ASSET__music3__mfdoom_mid, REPEAT_LOOP);
      return;
    case 11:
      play_song(&ASSET__music2__badapple_nointro_mid, REPEAT_LOOP);
      return;
    default:
      play_song(&ASSET__music3__e1m1_mid, REPEAT_LOOP);
      return;
  }
}

Instrument guitar = {
  0x6f, 0x40, 0x68, 0x5f, //initial amplitudes
  0x00, 0xFF, 0x02, 0x08, //decay rates
  0x00, 0x00, 0x40, 0x08, //sustain values
  12, 36, 0, 24, //per operator note offsets
  8, //feedback amount
  -12 //channel note offset
};

Instrument guitar2 = {
  0x60, 0x40, 0x88, 0x4f,
  0x00, 0xFF, 0x02, 0x01,
  0x00, 0x00, 0x40, 0x30,
  12, 36, 0, 24,
  8,
  -12
};

Instrument piano = {
  0x30, 0x40, 0x40, 0x5f,
  0x04, 0x02, 0x10, 0x02,
  0x04, 0x02, 0x10, 0x30,
  0, 0, 0, 0, 
  0,
  0
};

Instrument slapbass = {
  0x58, 0x88, 0x58, 0x5f,
  0x18, 0x08, 0x04, 0x02,
  0x18, 0x08, 0x04, 0x02,
  28, 12, 0, 12,
  0,
  -24
};

Instrument snare = {
  0x88, 0x8f, 0x8f, 0x38,
  0x18, 0x02, 0x04, 0x04,
  0x18, 0x08, 0x08, 0x04,
  36, 0, 0, 0,
  8,
  -8
};

int main () {
    char channel, op;
    char y;

    init_graphics();
    init_dynawave();
    init_music();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    load_instrument(0, &piano);
    load_instrument(1, &slapbass);
    load_instrument(2, &snare);
    load_instrument(3, &piano);

    song_number = 0;
    play_track(song_number);

    while (1) {                                     //  Run forever
        update_inputs();

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
          song_number = song_number + 1;
          if(song_number > MAX_SONG_ID) {
            song_number = 0;
          }
          play_track(song_number);
        } else if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
          if(song_number == 0) {
            song_number = MAX_SONG_ID;
          } else {
            song_number = song_number - 1;
          }
          play_track(song_number);
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
          load_instrument(0, &guitar);
        } else if(~player1_buttons & player1_old_buttons & INPUT_MASK_A) {
          load_instrument(0, &piano);
        }

        clear_screen(7);
        y = 16;
        for(channel = 0; channel < 4; ++channel) {
          for(op = 0; op < 4; ++op) {
            draw_box(1, y, audio_amplitudes[op + (channel << 2)], 2, 16);
            y += 4;
          }
          y += 6;
        }
        clear_border(0);
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        PROFILER_START(0);
        tick_music();
        PROFILER_END(0);
        
    }

  return (0);                                     //  We should never get here!
}