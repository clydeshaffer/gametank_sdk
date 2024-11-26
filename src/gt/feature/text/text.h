#ifndef TEXT_H
#define TEXT_H

#include "../../../gen/modules_enabled.h"

#ifndef ENABLE_MODULE_TEXT
#error "Module TEXT included but not enabled!"
#endif

#include "../../gfx/sprites.h"

#define TEXT_COLOR_BLACK 0
#define TEXT_COLOR_WHITE 128

void text_init();

SpriteSlot text_load_font();

void text_print_string(char* text);
void text_sprint_num(char* s, unsigned char num);

extern char text_cursor_x, text_cursor_y, text_print_width, text_print_line_start;
extern unsigned char text_color;

#endif
