#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/sfx.h"

#pragma data-name (push, "SAVE")
char saved_pos[4] = {30, 40, 1, 1};
#pragma data-name (pop)

char pos[4];

int main () {

    init_graphics();
    init_dynawave();
    init_music();

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

    while (1) { 
        clear_screen(3);         
        draw_box(1, 16, sfx_progress < 126 ? sfx_progress+1 : 126, 128-32, 92);                           //  Run forever
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
            play_sound_effect(&ASSET__sfx__sfx_djump_bin, 1);
        }
        sleep(1);
        tick_music();
        flip_pages();
        update_inputs();
    }

  return (0);                                     //  We should never get here!
}