#include "gt/gametank.h"
#include "gt/input.h"
#include "gt/audio/music.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gen/assets/music.h"
#include "gen/assets/sdk_default.h"

#include "ui/bittext.h"

char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

char last_tracker_line = 0;
char write_phase = 0;
extern char tracker_line_number;
extern char tracker_line_buf[33];
char scroll_pos = 0;

void main () {
    play_song(&ASSET__music__rudeb_bin, REPEAT_LOOP);
    
    bittext_color = 7;
    bittext_bg = 32;
    
    while (1) {                                     //  Run forever
        //queue_clear_screen(3);
	queue_draw_sprite(0, 0, 127, 127, 0, scroll_pos, 0);
        //queue_clear_border(0);
        
        box_x += dx;
        box_y += dy;
        if(box_x == 1) {
            dx = 1;
        } else if(box_x == 119) {
            dx = -1;
        }
        if(box_y == 8) {
            dy = 1;
        } else if(box_y == 112) {
            dy = -1;
        }

        if(player1_new_buttons & INPUT_MASK_UP) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 0);
        }
 
        if(player1_new_buttons & INPUT_MASK_DOWN) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 1);
        }
        
        if(player1_new_buttons & INPUT_MASK_LEFT) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 2);
        }
        
        if(player1_new_buttons & INPUT_MASK_RIGHT) {
            play_sound_effect(ASSET__music__sound_sfx_ID, 3);
        }

        if(player1_new_buttons & INPUT_MASK_START) {
            play_song(&ASSET__sdk_default__jingle_bin, REPEAT_RESUME);
        }

        PROFILER_START(0);
        tick_music();
        PROFILER_END(0);

        await_draw_queue();


    if(scroll_pos < 128) {
        direct_prepare_sprite_ram_array_mode(SPRITE_OFFSET_Y_MASK);
    } else {
        direct_prepare_sprite_ram_array_mode(0);
    }


	if(last_tracker_line != tracker_line_number) {
		last_tracker_line = tracker_line_number;
		tracker_line_buf[32] = 0;
		write_phase = 1;
	}
	if(write_phase == 1) {
		bittext_set_cursor(64, scroll_pos);
		bittext_write_str_opaque(tracker_line_buf+16);
		tracker_line_buf[16] = 0;
		write_phase = 2;
	} else if(write_phase == 2) {
		bittext_set_cursor(0, scroll_pos);
		bittext_write_str_opaque(tracker_line_buf);
		scroll_pos += 8;
		write_phase = 0;
		tracker_line_buf[16] = ' '; 
	}
        await_vsync(1);
        flip_pages();
        update_inputs();
        
    }
}
