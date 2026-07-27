#include "ps2.h"
#include "util.h"

#define MOUSE_SHIFT_BITS 2
char mouse_display_x = 64, mouse_display_y = 64;
int mouse_x = 64 << MOUSE_SHIFT_BITS, mouse_y = 64 << MOUSE_SHIFT_BITS;

char byteParity = 0;
char mouseStatus;
char oldMouseStatus;
int mouse_rel_x;
int mouse_rel_y;
char lastMouseRead = 0;

static char mouseChanged;
char update_mouse() {
    oldMouseStatus = mouseStatus;
    mouseChanged = 0;
    if(!ps2_data_ready()) {
        byteParity = 0;
    }
    while(ps2_data_ready()) {
        mouseChanged = 1;
        lastMouseRead = read_byte();

        mouse_display_x = mouse_x >> MOUSE_SHIFT_BITS;
        mouse_display_y = mouse_y >> MOUSE_SHIFT_BITS;

        if(byteParity == 0) {
            mouseStatus = lastMouseRead;
        } else if(byteParity == 1) {
            mouse_rel_x = lastMouseRead;
            if(mouseStatus & 16) {
                mouse_rel_x |= 0xFF00;
            }
            mouse_x += mouse_rel_x;
            if(mouse_x < 0) mouse_x = 0;
            if(mouse_x > (127 << MOUSE_SHIFT_BITS)) mouse_x = (127 << MOUSE_SHIFT_BITS);
        } else if(byteParity == 2) {
            mouse_rel_y = lastMouseRead;
            if(mouseStatus & 32) {
                mouse_rel_y |= 0xFF00;
            }
            mouse_y -= mouse_rel_y;
            if(mouse_y < 0) mouse_y = 0;
            if(mouse_y > (127 << MOUSE_SHIFT_BITS)) mouse_y = (127 << MOUSE_SHIFT_BITS);
        }

        byteParity++;
        if(byteParity == 3) byteParity = 0;
        delayMicroseconds(255);
        delayMicroseconds(255);
    }
    return mouseChanged;
}