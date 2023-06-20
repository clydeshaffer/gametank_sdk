#include "gametank.h"
#include "drawing_funcs.h"

int main () {
    char col = 30, row = 20;
    int dx = 1, dy = 1;

    flip_pages();

    clear_border(0);

    flip_pages();

    clear_border(0);

    while (1) {                                     //  Run forever
        clear_screen(3);
        FillRect(col, row, 8, 8, 92);
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
        
        flip_pages();
        sleep(1);
    }

  return (0);                                     //  We should never get here!
}