#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/gfx.h"
#include "banking.h"

#define BRICKS_LEFT_EDGE 16
#define BRICKS_RIGHT_EDGE 112
#define BRICKS_TOTAL_WIDTH 96
#define BRICKS_TOTAL_HEIGHT 24
#define BRICKS_TOP_EDGE 24
#define BRICKS_BOTTOM_EDGE 48
#define BRICKS_WIDTH 8
#define BRICKS_HEIGHT 4
#define BRICKS_WIDTH_SHIFT 3
#define BRICKS_HEIGHT_SHIFT 2

#define PLAYER_LIMIT_LEFT 17
#define PLAYER_LIMIT_RIGHT 111

int inputs = 0, last_inputs = 0;
int inputs2 = 0, last_inputs2 = 0;

void updateInputs(){
    char inputsA, inputsB;
    inputsA = *gamepad_2;
    inputsA = *gamepad_1;
    inputsB = *gamepad_1;

    last_inputs = inputs;
    inputs = ~((((int) inputsB) << 8) | inputsA);
    inputs &= INPUT_MASK_ALL_KEYS;

    inputsA = *gamepad_2;
    inputsB = *gamepad_2;
    last_inputs2 = inputs2;
    inputs2 = ~((((int) inputsB) << 8) | inputsA);
    inputs2 &= INPUT_MASK_ALL_KEYS;
}

char boxes[96];

char check_brick_hit(unsigned char x, unsigned char y) {
    x -= BRICKS_LEFT_EDGE;
    y -= BRICKS_TOP_EDGE;
    if(x >= BRICKS_TOTAL_WIDTH) return 0;
    if(y >= BRICKS_TOTAL_HEIGHT) return 0;
    x = x >> BRICKS_WIDTH_SHIFT;
    y = y >> BRICKS_HEIGHT_SHIFT;
    return (x + (y << 4)) + 1;
}

char check_player_hit(unsigned char bx, unsigned char px) {
    px -= bx;
    if(px > 127) {
        px = (~px)+1;
    }
    return (px < 18);
}

char setup_bricks() {
    unsigned char i,k;
    i = 0;
    for(k = 0; k < 96; ++k) {
        if((k & 15) <= 12) {
            boxes[k] = (i & 7) << 3;
            ++i;
        } else {
            boxes[k] = 0xFF;
        }
    }
}

int main () {
    char i, j, k;
    char ball_alive;
    unsigned char col = 64, row = 64;
    unsigned char dx = 1, dy = 255;
    unsigned char ticks = 0;
    char player_x = 64;
    char death_count = 0;
    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | frameflip;
    bankflip = BANK_SECOND_FRAMEBUFFER;
    *dma_flags = flagsMirror;
    banksMirror = bankflip;
    *bank_reg = banksMirror;

    init_dynawave();
    init_music();

    setup_bricks();

    load_spritesheet(&ASSET__gfx__ball_bmp, 0);
    load_spritesheet(&ASSET__gfx__player_bmp, 1);
    load_spritesheet(&ASSET__gfx__bricks_bmp, 2);
    load_spritesheet(&ASSET__gfx__background1_bmp, 3);

    flip_pages();

    clear_border(0);
    await_draw_queue();
    flip_pages();

    clear_border(0);
    await_draw_queue();
    ball_alive = 1;

    change_rom_bank(BANK_gfx);

    while (1) {
        updateInputs();
        draw_sprite_frame(
            &ASSET__gfx__background1_json_ptr,
            64, 64,
            0,
            SPRITE_FLIP_NONE,
            3,
            0
        );
        
        if(ball_alive) {
            col += dx;
            row += dy;
            if(col == 4) {
                dx = 1;
            } else if(col == 119) {
                dx = 255;
            }
            if(row == 16) {
                dy = 1;
            } else if((row == 108) && check_player_hit(col, player_x)) {
                dy = 255;
            } else if(row == 127) {
                ++death_count;
                ball_alive = 0;
                ticks = 0;
                do_noise_effect(80, -8, 10);
            }

            k = check_brick_hit(col, row);
            if(k && (boxes[k-1] != 0xFF)) {
                boxes[k-1] = 0xFF;
                do_noise_effect(95, 12, 4);
                if((col & 7) == 0) {
                    dx = 255;
                    col += dx;
                    col += dx;
                } else if((col & 7) == 7) {
                    dx = 1;
                    col += dx;
                    col += dx;
                }

                if((row & 3) == 0) {
                    dy = 255;
                    row += dy;
                    row += dy;
                } else if((row & 3) == 3) {
                    dy = 1;
                    row += dy;
                    row += dy;
                }
            }

            if(inputs & INPUT_MASK_LEFT) {
                --player_x;
            }

            if(inputs & INPUT_MASK_RIGHT) {
                ++player_x;
            }

            if(player_x < PLAYER_LIMIT_LEFT) player_x = PLAYER_LIMIT_LEFT;
            if(player_x > PLAYER_LIMIT_RIGHT) player_x = PLAYER_LIMIT_RIGHT;

            draw_sprite_frame(
                &ASSET__gfx__ball_json_ptr,
                col, row,
                (ticks >> 3) & 3,
                SPRITE_FLIP_NONE,
                0,
                0
            );
            draw_sprite_frame(
                &ASSET__gfx__player_json_ptr,
                player_x, 116,
                (ticks >> 3) % 6,
                SPRITE_FLIP_NONE,
                1,
                0
            );
            clear_border(0);
        } else {
            clear_border(0);
            if(ticks < 32) {
                draw_sprite_frame(
                    &ASSET__gfx__player_json_ptr,
                    player_x, 116,
                    (ticks >> 3) + 6,
                    SPRITE_FLIP_NONE,
                    1,
                    0
                );
            } else if(ticks == 255) {
                ball_alive = 1;
                col = 64;
                row = 64;
                dx = 1;
                dy = 255;
                player_x = 64;
                setup_bricks();
                if(death_count == 3) {
                    load_spritesheet(&ASSET__gfx__background2_bmp, 3);
                } else if(death_count == 4) {
                    load_spritesheet(&ASSET__gfx__background1_bmp, 3);
                }
            }
        }

        

        ++ticks;
        
        await_draw_queue();
        while(queue_pending != 0) {
            asm("CLI");
            wait();
        }

        k = 0;
        banksMirror = bankflip;
        banksMirror &= ~BANK_GRAM_MASK;
        banksMirror |= GRAM_PAGE(2);
        *bank_reg = banksMirror;
        flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | DMA_OPAQUE | frameflip | DMA_GCARRY;
        *dma_flags = flagsMirror;
        for(i = 24; i < 48; i += 4) {
            for(j = 16; j < 112; j += 8) {
                if(boxes[k] != 0xFF) {
                    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | DMA_OPAQUE | frameflip | DMA_GCARRY;
                    *dma_flags = flagsMirror;
                    draw_sprite_now(j, i, 8, 4, boxes[k], 0);
                }
                ++k;
            }
            k += 4;
        }

        flip_pages();
        tick_music();
        change_rom_bank(BANK_gfx);
        sleep(1);
    }

  return (0);                                     //  We should never get here!
}