#ifndef TEXT_H
#define TEXT_H

#define TEXT_COLOR_BLACK 0
#define TEXT_COLOR_WHITE 128

void init_text();

void load_font(char slot);

void print_text(char* text);

extern char text_cursor_x, text_cursor_y, text_print_width, text_print_line_start;
extern unsigned char text_color;

#endif
