#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "feature/text/text.h"
#include "dictionary.h"

#define KEYBOARD_X 24
#define KEYBOARD_Y 92

#define KEYBOARD_ROWS 3
const char keyboard_row_length[KEYBOARD_ROWS] = {10, 9, 7};
const char keyboard_letters[] = "QWERTYUIOP      ASDFGHJKL       ZXCVBNM         ";
char keyb_col;
char keyb_row;

#define TEXTBUF_SIZE (WORD_LENGTH*GUESS_LIMIT)+1
char textbuf[TEXTBUF_SIZE];
char box_colors[TEXTBUF_SIZE];
char textbuf_i;
char textbuf_word_offset;
char was_valid;
char r,c,i;
int main () {

    init_graphics();
    init_text();

    load_font(7);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    text_print_line_start = 32;
    text_print_width = 128 - 16;

    for(textbuf_i = 0; textbuf_i < TEXTBUF_SIZE; ++textbuf_i) {
        textbuf[textbuf_i] = 0;
        box_colors[textbuf_i] = 3;
    }

    textbuf_i = 0;
    textbuf_word_offset = 0;

    keyb_col = 0;
    keyb_row = 0;
    was_valid = 0;

    set_secret_word(13209);

    while (1) { 
        clear_screen(3);

        update_inputs();
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_UP) {
            if(keyb_row > 0) {
                --keyb_row;
            } else {
                keyb_row = KEYBOARD_ROWS-1;
            }
        }
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_DOWN) {
            if(keyb_row < KEYBOARD_ROWS-1) {
                ++keyb_row;
                if(keyb_col >= keyboard_row_length[keyb_row]) {
                    keyb_col = keyboard_row_length[keyb_row] - 1;
                }
            } else {
                keyb_row = 0;
            }
        }
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_LEFT) {
            if(keyb_col > 0) {
                --keyb_col;
            } else {
                keyb_col = keyboard_row_length[keyb_row]-1;
            }
        }
        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_RIGHT) {
            if(keyb_col < keyboard_row_length[keyb_row]-1) {
                ++keyb_col;
            } else {
                keyb_col = 0;
            }
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_A) {
            if(textbuf_i < WORD_LENGTH) {
                textbuf[textbuf_i+textbuf_word_offset] = keyboard_letters[(keyb_row * 16) + keyb_col];
                ++textbuf_i;
            }

            
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
            if(textbuf_i == WORD_LENGTH) {
                was_valid = 1 + lookup_word(textbuf + textbuf_word_offset);
                if(was_valid == 2) {

                    for(c = 0; c < WORD_LENGTH; ++c) {
                        if(textbuf[textbuf_word_offset + c] == 'A')
                            box_colors[textbuf_word_offset + c] = 52;
                        else if(textbuf[textbuf_word_offset + c] == 'B')
                            box_colors[textbuf_word_offset + c] = 20;
                    }

                    was_valid = 0;
                    textbuf_word_offset += 5;
                    textbuf_i = 0;
                }
            }
        }

        if(player1_buttons & ~player1_old_buttons & INPUT_MASK_B) {
            if(textbuf_i > 0) {
                --textbuf_i;
                textbuf[textbuf_i+textbuf_word_offset] = 0;
            }
            was_valid = 0;
        }

        draw_box((keyb_col << 3) + KEYBOARD_X + (keyb_row << 2), (keyb_row << 3) + KEYBOARD_Y, 8, 8, 7);

        i=0;
        for(r = 0; r < (8*GUESS_LIMIT); r += 8) {
            for(c = 0; c < (8 * WORD_LENGTH); c += 8) {
                draw_box(44 + c, 16 + r, 8, 8, box_colors[i++]);
            }
        }

        await_draw_queue();

        text_print_width = 128;
        text_use_alt_color = 0;
        text_cursor_x = KEYBOARD_X;
        text_cursor_y = KEYBOARD_Y;
        print_text("QWERTYUIOP");
        text_cursor_x = KEYBOARD_X+4;
        text_cursor_y = KEYBOARD_Y+8;
        print_text("ASDFGHJKL");
        text_cursor_x = KEYBOARD_X+8;
        text_cursor_y = KEYBOARD_Y+16;
        print_text("ZXCVBNM");

        text_print_line_start = 44;
        text_print_width = 8 * WORD_LENGTH;
        text_cursor_x = 44;
        text_cursor_y = 16;
        print_text(textbuf);
        if(textbuf_i < 5)
            print_text("_");

        text_print_width = 128;
        if(was_valid == 1) {
            text_cursor_x = 28;
            text_cursor_y = 64;
            print_text("not valid");
        } else if(was_valid == 2) {
            text_cursor_x = 44;
            text_cursor_y = 64;
            text_use_alt_color = 1;
            print_text("valid");
        } else {
            text_cursor_x = 44;
            text_cursor_y = 64;
            text_use_alt_color = 1;
            print_text(get_secret_word());
        }

        sleep(1);
        flip_pages();
        
    }

  return (0);
}