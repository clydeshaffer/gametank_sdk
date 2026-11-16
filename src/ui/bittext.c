#include "bittext.h"
#include "font.h"
#include "gametank.h"

pointer_t bittext_dest_u;
char bittext_color = 32;
char bittext_bg = 7;

#define dest_bytes bittext_dest_u.b

void bittext_write_char(char c) {
    char i, pat;
    int c_idx;
    c_idx = c - ' ';
    c_idx <<= 2;
    i = 1;
    if(c & 128) i = 0;
    for(; i < 5; ++i) {
        pat = system_font_monochrome[c_idx];
        if(pat & 1) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 4) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 16) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 64) *bittext_dest = bittext_color;
        dest_bytes.msb -= 3;
        bittext_dest += 128; 
        if(pat & 2) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 8) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 32) *bittext_dest = bittext_color;
        ++dest_bytes.msb;
        if(pat & 128) *bittext_dest = bittext_color;
        bittext_dest -= 895;
        ++c_idx;
    }
}

void bittext_write_char_opaque(char c) {
    char i, pat;
    int c_idx;
    c_idx = c - ' ';
    c_idx <<= 2;
    i = 1;
    if(c & 128) i = 0;
    for(; i < 5; ++i) {
        pat = system_font_monochrome[c_idx];
        *bittext_dest = (pat & 1) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 4) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 16) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 64) ? bittext_color : bittext_bg;
        dest_bytes.msb -= 3;
        bittext_dest += 128; 
        *bittext_dest = (pat & 2) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 8) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 32) ? bittext_color : bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = (pat & 128) ? bittext_color : bittext_bg;
        bittext_dest -= 895;
        ++c_idx;
    }
}

void bittext_erase_char() {
    char i;
    for(i=0; i < 4; ++i) {
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        dest_bytes.msb -= 3;
        bittext_dest += 128; 
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        ++dest_bytes.msb;
        *bittext_dest = bittext_bg;
        bittext_dest -= 895;
    }
}


void bittext_set_cursor(char x, char y) {
    //dest_bytes.msb = 0x40 | (y >> 1);
    //dest_bytes.lsb = x & 0x7F;
    //if(y & 1) dest_bytes.lsb |= 128;
    bittext_set_x(x);
    bittext_set_y(y & 127);
}

void bittext_write_str_opaque(char* c) {
    while(*c != 0) {
        PROFILER_START(0);
        bittext_draw_char(*c);
        PROFILER_END(0);
        ++c;
    }
}
