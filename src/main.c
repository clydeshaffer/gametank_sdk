#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/music.h"
#include "gen/assets/music2.h"
#include "input.h";

char song_number = 0;

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
    default:
      play_song(&ASSET__music__badapple_mid, REPEAT_LOOP);
      return;
  }
}

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

    song_number = 0;
    play_track(song_number);

    while (1) {                                     //  Run forever
        update_inputs();

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
          song_number = song_number + 1;
          if(song_number > 9) {
            song_number = 0;
          }
          play_track(song_number);
        } else if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
          if(song_number == 0) {
            song_number = 9;
          } else {
            song_number = song_number - 1;
          }
          play_track(song_number);
        }

        clear_screen(7);
        y = 16;
        for(channel = 0; channel < 4; ++channel) {
          for(op = 0; op < 4; ++op) {
            draw_box(1, y + (op << 2), audio_amplitudes[(op << 2) + channel], 2, 16);
          }
          y += 24;
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