#include "gametank.h"
#include "drawing_funcs.h"
#include "feature/text/text.h"
#include "games/snake.h"

int main () {
    char col = 30, row = 20;
    int dx = 1, dy = 1;

    init_graphics();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    init_text();
    load_font(7);

    run_snake_game();

    while (1) {                                     //  Run forever
        clear_screen(3);
        draw_box(col, row, 8, 8, 92);
        col += dx;
        row += dy;
        if(col == 1) {
            dx = 1;
        } else if(col == 119) {
            dx = -1;
        }
        if(row == 8) {
            dy = 1;
        } else if(row == 112) {
            dy = -1;
        }
        
        await_draw_queue();
        sleep(1);
        flip_pages();
        
    }

  return (0);                                     //  We should never get here!
}