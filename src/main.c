#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/feature/text/text.h"
#include "gt/gfx/draw_direct.h"
#include "gt/gfx/sprites.h"
#include "gen/assets/gfx.h"

char box_x = 0, box_y = 16;
char dx = 1, dy = 1;
int mouse_x = 64 << 3, mouse_y = 64 << 3;

#define MAX_ICONS 5
#define MOUSE_SHIFT_BITS 2
char icons_x[MAX_ICONS] = { 24, 64, 72, 32, 80};
char icons_y[MAX_ICONS] = { 24, 64, 36, 72, 90};
char icons_f[MAX_ICONS] = {  3,  1,  2,  1,  2};
char dragging_index = 255;
char drag_rel_x = 0;
char drag_rel_y = 0;

#define CLEAR_INHIBIT (1<<3)
#define WRITE_STROBE (1<<4)
#define READ_STROBE (1<<5)
#define PACKET_DONE (1<<6)

#define SET(bit) via[ORA] |= bit
#define CLEAR(bit) via[ORA] &= ~bit

char box_color = 92;

SpriteSlot text_sprite;
SpriteSlot bg_sprite;
SpriteSlot icons_sprite;
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
char ledMask = 0;
char oldLedMask = 0;
char isMouse = 0;
char byteParity = 0;
char mouseStatus;
char oldMouseStatus;
int mouse_rel_x;
int mouse_rel_y;
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

static char tmp;
void delayMicroseconds(char c) {
    tmp = c;
    while(tmp) {
        --tmp;
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

//0, 1, 2, 3, 4, 5, 6, 7
//0, 7, 6, 5, 4, 1, 2, 3
//only needed on the breadboard
//which was wired for physical not digital convenience
//remove for PCB version with consistent ordering
char scramble(char b) {
    tmp = 0;
    tmp |= (b & 0b10001000);
    //tmp |= (b & 1) << 6;
    //tmp |= (b & 2) << 4;
    //tmp |= (b & 4) << 2;
    //tmp |= (b & (16|32|64)) >> 4;
    tmp |= (b & (1|2|4)) << 4;
    tmp |= (b & 16) >> 2;
    tmp |= (b & 32) >> 4;
    tmp |= (b & 64) >> 6;
    return tmp;
}

char read_byte() {
    CLEAR(READ_STROBE);
    SET(READ_STROBE);
    CLEAR(READ_STROBE);
    tmp = via[ORB];
    SET(READ_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    return tmp;
}

void send_byte(char b) {
    via[ORA] |= PACKET_DONE;
    via[DDRA] |= PACKET_DONE;
    via[DDRB] = 0xFF;
    via[ORB] = scramble(b);
    delayMicroseconds(100);
    CLEAR(WRITE_STROBE);
    via[DDRA] &= ~PACKET_DONE;
    via[ORA] &= ~PACKET_DONE;;
    SET(WRITE_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    via[DDRB] = 0;
}

char wait_for_packet() {
  int cnt = 0;
  while(!(via[ORA] & PACKET_DONE)) {
    delayMicroseconds(100);
    ++cnt;
    if(cnt > 500) {
      return 0;
    }
  }
  return 1;
}

void clear_clock_inhibit() {
  CLEAR(CLEAR_INHIBIT);
  SET(CLEAR_INHIBIT);
}

int send_byte_and_get_response(unsigned char b, unsigned char* buf, int lim) {
  int response_bytes = 0;

  send_byte(b);
  wait_for_packet();
  lastRead = read_byte();
  clear_clock_inhibit();
  while((response_bytes < lim) && wait_for_packet()) {
    lastRead = read_byte();
    clear_clock_inhibit();
    buf[response_bytes] = lastRead;
    response_bytes++;
  }
  return response_bytes;
}

void handle_led_keys(unsigned char lastRead) {
    if((lastRead & 0xFF) == 0x77) {
        ledMask ^= 2;
    }

    if((lastRead & 0xFF) == 0x58) {
        ledMask ^= 4;
    }

    if((lastRead & 0xFF) == 0x7E) {
        ledMask ^= 1;
    }

    if(ledMask != oldLedMask) {
        oldLedMask = ledMask;
        send_byte(0xED);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
        clear_clock_inhibit();
        wait_for_packet();
        clear_clock_inhibit();
        send_byte(ledMask);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
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

void mainloop_keyboard () {
    
    while (1) {                                     //  Run forever
        queue_clear_border(0);
        
        /*box_x += dx;
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
        }*/
 
        while(via[ORA] & PACKET_DONE) {
            via[ORA] &= ~READ_STROBE;
            via[ORA] |= READ_STROBE;
            via[ORA] &= ~READ_STROBE;
            lastRead = via[ORB];
            via[ORA] |= READ_STROBE;
            via[ORA] &= ~CLEAR_INHIBIT;
            via[ORA] |= CLEAR_INHIBIT;

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
            box_x = text_cursor_x;
            box_y = text_cursor_y;
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
            text_cursor_x = box_x;
            text_cursor_y = box_y;
            queue_draw_box(text_cursor_x, text_cursor_y, 8, 8, 32);
        }
    
        await_draw_queue();

        if(printbuf[0]) {{
            text_cursor_x = box_x;
            text_cursor_y = box_y;
            if(printbuf[0] == '\n') text_cursor_x = 0;
            text_print_string(printbuf);
            text_back();
            printbuf[0] = 0;
        }}
        
        //queue_draw_box(0, 100, 127, 10, box_color);
        queue_clear_border(0);
        await_draw_queue();
        await_vsync(1);
        flip_pages();
 
    }
}

char cabs(char x) {
    if(x & 128) return -x;
    return x;
}

void mainloop_mouse() {

    bg_sprite = allocate_sprite(&ASSET__gfx__desktop_bmp_load_list);
    icons_sprite = allocate_sprite(&ASSET__gfx__icons_bmp_load_list);
    set_sprite_frametable(icons_sprite, ASSET__gfx__icons_json);

    while(1) {

        //queue_clear_screen(0);
        queue_draw_sprite(0,0,127,127,0,0,bg_sprite);

        if(!via[ORA] & PACKET_DONE) {
            byteParity = 0;
        }
        while(via[ORA] & PACKET_DONE) {
            lastRead = read_byte();

            box_x = mouse_x >> MOUSE_SHIFT_BITS;
            box_y = mouse_y >> MOUSE_SHIFT_BITS;

            if(byteParity == 0) {
                mouseStatus = lastRead;

                if(mouseStatus & ~oldMouseStatus & 1) {
                    for(tmp = 0; tmp < MAX_ICONS; tmp++) {
                        if(icons_f[tmp]) {
                            if(cabs(icons_x[tmp] - box_x) < 8) {
                                if(cabs(icons_y[tmp] - box_y) < 8) {
                                    dragging_index = tmp;
                                    drag_rel_x = icons_x[tmp] - box_x;
                                    drag_rel_y = icons_y[tmp] - box_y;
                                    break;
                                }
                            }
                        }
                    }
                } else if(~mouseStatus & oldMouseStatus & 1) {
                    if(dragging_index != 0) {
                         if(cabs(icons_x[dragging_index] - icons_x[0]) < 8) {
                            if(cabs(icons_y[dragging_index] - icons_y[0]) < 8) {
                                icons_f[dragging_index] = 0;
                            }
                        }   
                    }
                    dragging_index = 255;
                }

                oldMouseStatus = mouseStatus;
            } else if(byteParity == 1) {
                mouse_rel_x = lastRead;
                if(mouseStatus & 16) {
                    mouse_rel_x |= 0xFF00;
                }
                mouse_x += mouse_rel_x;
                if(mouse_x < 0) mouse_x = 0;
                if(mouse_x > (127 << MOUSE_SHIFT_BITS)) mouse_x = (127 << MOUSE_SHIFT_BITS);
            } else if(byteParity == 2) {
                mouse_rel_y = lastRead;
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

        if(dragging_index != 255) {
            icons_x[dragging_index] = drag_rel_x + box_x;
            icons_y[dragging_index] = drag_rel_y + box_y;
        }

        for(tmp = 0; tmp < MAX_ICONS; ++tmp) {
            if(icons_f[tmp]) {
                queue_draw_sprite_frame(icons_sprite, icons_x[tmp], icons_y[tmp], icons_f[tmp], 0);
            }
        }
        queue_draw_sprite_frame(icons_sprite, box_x, box_y, 0, 0);
        queue_clear_border(0);
        await_draw_queue();
        await_vsync(1);
        flip_pages();
    }
}

void main () {

    via[DDRA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
    via[DDRA] &= ~PACKET_DONE;
    via[ORA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
    via[DDRB] = 0;

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