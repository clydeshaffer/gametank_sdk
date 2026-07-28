#include "gt/gametank.h"
#include "ps2.h"
#include "util.h"
#include "keyboard.h"
#include "mouse.h"
#include "desktop.h"
#include "mem.h"
#include "gt/gfx/draw_queue.h"
#include "gt/feature/text/text.h"
#include "gt/gfx/draw_direct.h"
#include "gt/gfx/sprites.h"
#include "banking.h"
#include "gen/bank_nums.h"

static char tmp = 0;

SpriteSlot text_sprite;

extern const char ps2_set2_to_char[256];
extern const char ps2_set2_to_upper_char[256];

#define LINE_MAX 64
char line_buf[64];
char debug_buf[256];
char debug_buf_idx = 0;
char line_buf_idx;
char response_buf[8] = {0};
char printbuf[8] = {0, '_', 0, 0, 0, 0, 0, 0};
char isBreak = 0;
char lastRead = 0;
char shiftMask = 0;
char isMouse = 0;
char old_text_x, old_text_y;
char resb;
char doubleclear = 0;


char hexchar(char n) {
    n = n & 0x0F;
    if(n < 10) {
        return '0' + n; 
    } else {
        return 'A' + (n - 10);
    }
}


char str_equals(char *c1, char *c2) {
    while((*c1 != 0) && (*c1 == *c2)) {
        c1++;
        c2++;
    }
    return *c1 == *c2;
}

void to_lower(char *c) {
    while(*c != 0) {
        if((*c <= 'Z') && (*c >= 'A')) {
            *c += ('a' - 'A');
        }
        c++;
    }
}



void print_hex_to_debug_buf(char c) {
    debug_buf[debug_buf_idx++] = hexchar(c >> 4);
    debug_buf[debug_buf_idx++] = hexchar(c);
    debug_buf[debug_buf_idx++] = ' ';
    debug_buf[debug_buf_idx++] = ' ';
    debug_buf[debug_buf_idx] = 0;
}

char di;
void debug_print_resp() {
    for(di = 0; di < resb; ++di) {
        print_hex_to_debug_buf(response_buf[di]);
    }
    debug_buf[debug_buf_idx++] = '|';
    debug_buf[debug_buf_idx++] = ' ';
    debug_buf[debug_buf_idx] = 0;
}

#pragma code-name (push, "PROG0")
void mainloop_keyboard () {

    while (1) {                                     //  Run forever
        queue_clear_border(0);
        
        while(ps2_data_ready()) {
            lastRead = read_byte();

            if(lastRead == 0xF0) {
                isBreak = 1;
            } else {
                if(!isBreak) {
                    if(!!shiftMask ^ !!(ledMask&4)) {
                        printbuf[0] = ps2_set2_to_upper_char[lastRead];
                    } else {
                        printbuf[0] = ps2_set2_to_char[lastRead];
                    }
                    
                    //printbuf[0] = hexchar(lastRead >> 4);
                    //printbuf[1] = hexchar(lastRead);
                    handle_led_keys(lastRead);
                }

                if(lastRead == 0x12) {
                    shiftMask &= ~1;
                    shiftMask |= !isBreak;
                } else if(lastRead == 0x59) {
                    shiftMask &= ~2;
                    shiftMask |= !isBreak << 1;
                }

                isBreak = 0;
            }
        }

        if(printbuf[0]) {
            queue_draw_box(text_cursor_x, text_cursor_y, 8, 8, 32);
            if(printbuf[0] == '\n') {
                line_buf[line_buf_idx] = 0;
                to_lower(line_buf);
                if(str_equals(line_buf, "clear")) {
                    queue_clear_screen(0);
                    doubleclear = 1;
                    text_cursor_x = 0;
                    text_cursor_y = 8;
                }
                line_buf_idx = 0;
            } else {
                if(line_buf_idx < (LINE_MAX-1)) {
                    line_buf[line_buf_idx] = printbuf[0];
                    ++line_buf_idx;
                }
            }
        }
    
        await_draw_queue();


        if(printbuf[0]) {{
            old_text_x = text_cursor_x;
            old_text_y = text_cursor_y;
            if(printbuf[0] == '\n') text_cursor_x = 0;
            text_print_string(printbuf);

        }}

        await_vsync(1);
        flip_pages();

        if(doubleclear) {
            queue_clear_screen(0);
            doubleclear = 0;
        }

        if(printbuf[0]) {
            text_cursor_x = old_text_x;
            text_cursor_y = old_text_y;
            queue_draw_box(text_cursor_x, text_cursor_y, 8, 8, 32);
        }
    
        await_draw_queue();

        if(printbuf[0]) {{
            text_cursor_x = old_text_x;
            text_cursor_y = old_text_y;
            if(printbuf[0] == '\n') text_cursor_x = 0;
            text_print_string(printbuf);
            text_back();
            printbuf[0] = 0;
        }}
        
        queue_clear_border(0);
        await_draw_queue();
        await_vsync(1);
        flip_pages();
 
    }
}
#pragma code-name(pop)

void mainloop_mouse() {

    desktop_init();

    while(1) {

        desktop_early_draw();

        update_mouse();
        desktop_update();
        desktop_draw();
        desktop_late_update();

        await_vsync(1);
        flip_pages();
    }
}

void main () {
    change_rom_bank(BANK_PROG0);

    mem_init();
    
    ps2_init();

    resb = send_byte_and_get_response(0xFF, response_buf, 8);
    debug_print_resp();

    delayMicroseconds(100);

    resb = send_byte_and_get_response(0xF5, response_buf, 8);
    debug_print_resp();

    delayMicroseconds(100);

    resb = send_byte_and_get_response(0xF2, response_buf, 8);
    if((resb == 2) && ((response_buf[1] == 0) || (response_buf[1] == 3) || (response_buf[1] == 4))) {
        isMouse = 1;
    }

    debug_print_resp();

    delayMicroseconds(100);

    resb = send_byte_and_get_response(0xF4, response_buf, 8);
    debug_print_resp();

    
    delayMicroseconds(100);

    text_init();
    text_sprite = text_load_font();
    text_cursor_y = 16;
    text_color = TEXT_COLOR_WHITE;

    text_print_string("GTOS v0.1\n\rReady\n\r");
    text_print_string(debug_buf);
    text_print_string("\n\r-");
    await_drawing();
    text_cursor_x = 0;
    text_cursor_y = 16;
    await_vsync(1);
    flip_pages();
    text_print_string("GTOS v0.1\n\rReady\n\r");
    text_print_string(debug_buf);
    text_print_string("\n\r-");
    await_drawing();
    await_vsync(1);
    flip_pages();

    debug_buf_idx = 0;
    debug_buf[0] = 0;

    if(isMouse) {
        text_print_string("Mouse detected\r\nLoading GUI...");
        await_drawing();
        flip_pages();
        mainloop_mouse();
    } else {
        mainloop_keyboard();
    }
}