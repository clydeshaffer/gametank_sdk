#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"
#include "dynawave.h"
#include "gen/assets/audiodata.h"

#pragma data-name (push, "SAVE")
char saved_pos[4] = {30, 40, 1, 1};
#pragma data-name (pop)

char pos[4];
char* audio_data_cursor;
int audio_data_counter;

int main () {
    static char i;

    init_graphics();
    init_dynawave();

    set_audio_param(PITCH_MSB+0, 0x00);
    set_audio_param(PITCH_LSB+0, 0x00);
    set_audio_param(PITCH_MSB+1, 0x00);
    set_audio_param(PITCH_LSB+1, 0x00);
    set_audio_param(PITCH_MSB+2, 0x00);
    set_audio_param(PITCH_LSB+2, 0x00);
    set_audio_param(PITCH_MSB+3, 0x00);
    set_audio_param(PITCH_LSB+3, 0x00);
    set_audio_param(PITCH_MSB+4, 0x00);
    set_audio_param(PITCH_LSB+4, 0x00);
    set_audio_param(PITCH_MSB+5, 0x00);
    set_audio_param(PITCH_LSB+5, 0x00);
    set_audio_param(PITCH_MSB+6, 0x00);
    set_audio_param(PITCH_LSB+6, 0x00);
    set_audio_param(PITCH_MSB+7, 0x00);
    set_audio_param(PITCH_LSB+7, 0x00);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);


    change_rom_bank(SAVE_BANK_NUM);
    pos[0] = saved_pos[0];
    pos[1] = saved_pos[1];
    pos[2] = saved_pos[2];
    pos[3] = saved_pos[3];

    audio_data_cursor = &ASSET__audiodata__pacer_bin_ptr;
    audio_data_counter = 0;

    while (1) {                                     //  Run forever
        clear_screen(3);

        change_rom_bank(ASSET__audiodata__pacer_bin_bank);
        for(i = 0; i < 8; ++i) {
            push_audio_param(PITCH_LSB+i, *audio_data_cursor);
            ++audio_data_cursor;
            push_audio_param(PITCH_MSB+i, *audio_data_cursor);
            ++audio_data_cursor;
            audio_data_counter += 2;
        }
        flush_audio_params();
        pop_rom_bank();

        if(audio_data_counter == 7792) {
            audio_data_cursor = &ASSET__audiodata__pacer_bin_ptr;
            audio_data_counter = 0;
        }

        draw_box(pos[1], pos[0], 8, 8, 92);
        pos[1] += pos[2];
        pos[0] += pos[3];
        if(pos[1] == 1) {
            pos[2] = 1;
        } else if(pos[1] == 119) {
            pos[2] = -1;
        }
        if(pos[0] == 8) {
            pos[3] = 1;
        } else if(pos[0] == 112) {
            pos[3] = -1;
        }
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        update_inputs();
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
          clear_save_sector();
          save_write(&pos, &saved_pos, sizeof(pos));
        }

    }

  return (0);                                     //  We should never get here!
}