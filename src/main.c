#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "feature/text/text.h"
#include "dictionary.h"
#include "gen/assets/gfx.h"

#define KEYBOARD_X 24
#define KEYBOARD_Y 92

#define KEYBOARD_ROWS 3
const char keyboard_row_length[KEYBOARD_ROWS] = {10, 9, 7};
const char keyboard_row_offset[KEYBOARD_ROWS] = {0, 4, 8};
const char keyboard_letters[] = "QWERTYUIOP      ASDFGHJKL       ZXCVBNM         ";
char keyboard_colors[ALPHABET_SIZE];
char keyb_col;
char keyb_row;

#define TEXTBUF_SIZE (WORD_LENGTH*GUESS_LIMIT)+1
char textbuf[TEXTBUF_SIZE];
char box_colors[TEXTBUF_SIZE];
char textbuf_i;
char textbuf_word_offset;

#define STATE_NONE 0
#define STATE_INVALID 1
#define STATE_VALID 2
#define STATE_WIN 3
#define STATE_LOSE 4
#define STATE_RESTART 5
char guess_state;

char r,c,i;
unsigned int word_index;
int main () {

    init_graphics();
    init_text();

    load_font(7);
    load_spritesheet(&ASSET__gfx__bg_bmp, 0);

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);

    while (1) {
        text_print_width = 128 - 8;

        for(textbuf_i = 0; textbuf_i < ALPHABET_SIZE; ++textbuf_i) {
            keyboard_colors[textbuf_i] = LETTER_COLOR_DEFAULT;
        }

        for(textbuf_i = 0; textbuf_i < TEXTBUF_SIZE; ++textbuf_i) {
            textbuf[textbuf_i] = 0;
            box_colors[textbuf_i] = 1;
        }

        textbuf_i = 0;
        textbuf_word_offset = 0;

        keyb_col = 0;
        keyb_row = 0;
        guess_state = STATE_NONE;
        update_inputs();
        draw_sprite(0, 0, 127, 127, 0, 0, 0);
        await_draw_queue();
        text_print_line_start = 44;
        text_cursor_x = 44;
        text_cursor_y = 16;
        text_use_alt_color = 1;
        print_text("Word\n\rGuess\n\rGame");
        text_cursor_x = 20;
        text_cursor_y = KEYBOARD_Y;
        text_use_alt_color = 0; 
        print_text("Press Start");
        sleep(1);
        flip_pages();
        while(!(player1_buttons & ~player1_old_buttons & INPUT_MASK_START)) {
            ++word_index;
            update_inputs();
        }

        set_secret_word(word_index);

        while (guess_state != STATE_RESTART) {
            ++word_index;
            draw_sprite(0, 0, 127, 127, 0, 0, 0);

            update_inputs();

            if(guess_state < STATE_WIN) {
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_UP) {
                    if(keyb_row > 0) {
                        --keyb_row;
                    } else {
                        keyb_row = KEYBOARD_ROWS-1;
                        if(keyb_col >= keyboard_row_length[keyb_row]) {
                            keyb_col = keyboard_row_length[keyb_row] - 1;
                        }
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
                        guess_state = 1 + lookup_word(textbuf + textbuf_word_offset);
                        if(guess_state == STATE_VALID) {

                            if(check_guess(textbuf + textbuf_word_offset, box_colors + textbuf_word_offset, keyboard_colors)) {
                                guess_state = STATE_WIN;
                            } else {
                                guess_state = STATE_NONE;
                                textbuf_word_offset += WORD_LENGTH;
                                textbuf_i = 0;
                                if(textbuf_word_offset == WORD_LENGTH*6) {
                                    guess_state = STATE_LOSE;
                                }
                            }
                        }
                    }
                }

                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_B) {
                    if(textbuf_i > 0) {
                        --textbuf_i;
                        textbuf[textbuf_i+textbuf_word_offset] = 0;
                    }
                    guess_state = STATE_NONE;
                }

                i=0;
                for(r = 0; r < KEYBOARD_ROWS; ++r) {
                    for(c = 0; c < (8 * keyboard_row_length[r]); c += 8) {
                        draw_box(KEYBOARD_X + c + keyboard_row_offset[r], KEYBOARD_Y + (r << 3), 8, 8, keyboard_colors[i++]);
                    }
                }
                draw_box((keyb_col << 3) + KEYBOARD_X + (keyb_row << 2), (keyb_row << 3) + KEYBOARD_Y, 8, 8, 7);
            } else {
                if(player1_buttons & ~player1_old_buttons & INPUT_MASK_START) {
                    guess_state = STATE_RESTART;
                }
            }

            i=0;
            for(r = 0; r < (8*GUESS_LIMIT); r += 8) {
                for(c = 0; c < (8 * WORD_LENGTH); c += 8) {
                    draw_box(44 + c, 16 + r, 8, 8, box_colors[i++]);
                }
            }

            await_draw_queue();

            if(guess_state < STATE_WIN) {

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
            }

            text_use_alt_color = 1;
            text_print_line_start = 44;
            text_print_width = 8 * WORD_LENGTH;
            text_cursor_x = 44;
            text_cursor_y = 16;
            print_text(textbuf);

            if(guess_state == STATE_INVALID) {
                text_print_width = 128;
                text_cursor_x = 28;
                text_cursor_y = 72;
                print_text("not valid");
            } else if(guess_state == STATE_WIN) {
                text_print_width = 128;
                text_cursor_x = 28;
                text_cursor_y = 72;
                text_use_alt_color = 1;
                print_text("Good job!");
                text_cursor_x = 20;
                text_cursor_y = KEYBOARD_Y+16;
                text_use_alt_color = 0; 
                print_text("Press Start");
            } else if(guess_state == STATE_LOSE) {
                text_print_width = 128;
                text_cursor_x = 4;
                text_cursor_y = 72;
                text_use_alt_color = 1;
                text_print_line_start = 4;
                print_text("    Too bad!\n\rIt was ");
                print_text(get_secret_word());
                print_text(" :(");
                text_cursor_x = 20;
                text_cursor_y = KEYBOARD_Y+16;
                text_use_alt_color = 0; 
                print_text("Press Start");
            } else if(textbuf_i < 5)
                print_text("_");

            sleep(1);
            flip_pages();
            
        }
    }

  return (0);
}