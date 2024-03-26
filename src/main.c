#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "persist.h"
#include "banking.h"

#pragma data-name (push, "SAVE")
char saved_pos[4] = {30, 40, 1, 1};
#pragma data-name (pop)

char pos[4];

int main () {

    init_graphics();

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

    while (1) {                                     //  Run forever
        clear_screen(3);
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