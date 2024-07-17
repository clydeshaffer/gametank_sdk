#include "gt/drawing_funcs.h"
#include "gt/banking.h"

extern int tanks_main();
extern int tetris_main();

char auto_tick_music = 0;

int main() {
    auto_tick_music = 0;
    //tanks_main();
    tetris_main();
    return 0;
}