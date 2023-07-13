#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "input.h"

#include "feature/text/text.h"
#include "games/snake.h"
#include "games/invaders.h"
#include "games/runner.h"
#include "gen/assets/gfx.h"

static char game_select;

int main () {

    init_graphics();
    init_dynawave();
    init_music();

    load_spritesheet(&ASSET__gfx__main_menu_bmp, 0);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    init_text();
    load_font(7);
    game_select = 0;

    while (1) {
        update_inputs();
        draw_sprite(0, 0, 127, 127, 0, 0, 0);
        clear_border(0);

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_UP) {
          if(game_select > 0) --game_select;
        } else if(player1_buttons & ~player1_old_buttons & INPUT_MASK_DOWN) {
          if(game_select < 4) ++game_select;
        } else if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_A | INPUT_MASK_START)) {
          if(game_select == 0) run_snake_game();
          else if(game_select == 1) run_invaders_game();
          else if(game_select == 2) run_runner_game();
        }

        await_draw_queue();

        text_cursor_x = 32;
        text_cursor_y = 72;
        text_print_line_start = 32;
        text_print_width = 96;
        print_text("Snake\n\rInvaders\n\rRed's Run\n\rTBD\n\rTBD");

        text_cursor_x = 24;
        text_cursor_y = 72 + (game_select << 3);
        print_text(">");

        sleep(1);
        flip_pages();
        
    }

  return (0);                                     //  We should never get here!
}