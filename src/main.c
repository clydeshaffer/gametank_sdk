#include "gametank.h"
#include "drawing_funcs.h"
#include "dynawave.h"
#include "music.h"
#include "gen/assets/gfx.h"
#include "input.h"

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

void setup_bricks() {
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
    char ball_x = 64, ball_y = 64;
    char ball_dx = 1, ball_dy = 255;
    char global_tick_counter = 0;
    char player_x = 64;
    char death_count = 0;

    init_graphics();
    init_dynawave();
    init_music();

    setup_bricks();

    load_spritesheet(&ASSET__gfx__ball_bmp, 0);
    load_spritesheet(&ASSET__gfx__player_bmp, 1);
    load_spritesheet(&ASSET__gfx__bricks_bmp, 2);
    load_spritesheet(&ASSET__gfx__background1_bmp, 3);

    init_graphics();

    flip_pages();
    clear_border(0);
    await_draw_queue();
    flip_pages();
    await_draw_queue();
    clear_border(0);
    await_draw_queue();
    ball_alive = 1;

    while (1) {
        update_inputs();
        draw_sprite_frame(
            &ASSET__gfx__background1_json,
            64, 64,
            0,
            SPRITE_FLIP_NONE,
            3
        );
        
        if(ball_alive) {
            ball_x += ball_dx;
            ball_y += ball_dy;
            if(ball_x == 4) {
                ball_dx = 1;
            } else if(ball_x == 119) {
                ball_dx = 255;
            }
            if(ball_y == 16) {
                ball_dy = 1;
            } else if((ball_y == 108) && check_player_hit(ball_x, player_x)) {
                ball_dy = 255;
            } else if(ball_y == 127) {
                ++death_count;
                ball_alive = 0;
                global_tick_counter = 0;
                do_noise_effect(80, -8, 10);
            }

            k = check_brick_hit(ball_x, ball_y);
            if(k && (boxes[k-1] != 0xFF)) {
                boxes[k-1] = 0xFF;
                do_noise_effect(95, 12, 4);
                if((ball_x & 7) == 0) {
                    ball_dx = 255;
                    ball_x += ball_dx;
                    ball_x += ball_dx;
                } else if((ball_x & 7) == 7) {
                    ball_dx = 1;
                    ball_x += ball_dx;
                    ball_x += ball_dx;
                }

                if((ball_y & 3) == 0) {
                    ball_dy = 255;
                    ball_y += ball_dy;
                    ball_y += ball_dy;
                } else if((ball_y & 3) == 3) {
                    ball_dy = 1;
                    ball_y += ball_dy;
                    ball_y += ball_dy;
                }
            }

            if(player1_buttons & INPUT_MASK_LEFT) {
                --player_x;
            }

            if(player1_buttons & INPUT_MASK_RIGHT) {
                ++player_x;
            }

            if(player_x < PLAYER_LIMIT_LEFT) player_x = PLAYER_LIMIT_LEFT;
            if(player_x > PLAYER_LIMIT_RIGHT) player_x = PLAYER_LIMIT_RIGHT;

            draw_sprite_frame(
                &ASSET__gfx__ball_json,
                ball_x, ball_y,
                (global_tick_counter >> 3) & 3,
                SPRITE_FLIP_NONE,
                0
            );
            draw_sprite_frame(
                &ASSET__gfx__player_json,
                player_x, 116,
                (global_tick_counter >> 3) % 6,
                SPRITE_FLIP_NONE,
                1
            );
            clear_border(0);
        } else {
            clear_border(0);
            if(global_tick_counter < 32) {
                draw_sprite_frame(
                    &ASSET__gfx__player_json,
                    player_x, 116,
                    (global_tick_counter >> 3) + 6,
                    SPRITE_FLIP_NONE,
                    1
                );
            } else if(global_tick_counter == 255) {
                ball_alive = 1;
                ball_x = 64;
                ball_y = 64;
                ball_dx = 1;
                ball_dy = 255;
                player_x = 64;
                setup_bricks();
                if(death_count == 3) {
                    load_spritesheet(&ASSET__gfx__background2_bmp, 3);
                } else if(death_count == 4) {
                    load_spritesheet(&ASSET__gfx__background1_bmp, 3);
                }
            }
        }

        ++global_tick_counter;
        
        await_draw_queue();

        k = 0;
        *dma_flags = flagsMirror;
        for(i = 24; i < 48; i += 4) {
            for(j = 16; j < 112; j += 8) {
                if(boxes[k] != 0xFF) {
                    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | DMA_OPAQUE | frameflip | DMA_GCARRY;
                    *dma_flags = flagsMirror;
                    draw_sprite_now(j, i, 8, 4, boxes[k], 0, GRAM_PAGE(2));
                }
                ++k;
            }
            k += 4;
        }

        sleep(1);
        flip_pages();
        tick_music();
    }

  return (0);                                     //  We should never get here!
}