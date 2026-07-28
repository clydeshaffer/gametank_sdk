#include "gametank.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../mouse.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"
#include "../gt/feature/random/random.h"

#pragma code-name (push, "PROG0")

#define SQUARE_COUNT 64
#define WAS_RIGHT_CLICK 0x80
#define SQUARE_REVEALED 0x80
#define SQUARE_FLAGGED 0x40
#define MINE_SQUARE 9
#define MINE_COUNT 10

typedef struct {
    char squares[SQUARE_COUNT];
    char square_clicked;
    char click_count;
    char game_done;
} mines_app_context;

#define ctx ((mines_app_context*)my(window_context))

static char tmp, tmp2;
char mines_app_open_count = 0;

#define mines_app_W 66
#define mines_app_H 77

char* addr_for_tile(char tile) {
    return vram + 66 + (tile << 10);
}

char* addr_for_square(char sq) {
    return vram + ((sq & 7) << 3) + ((sq & 0x38) << 7) + 1537;
}

void copy8x8(char* src, char* dest) {
    for(tmp = 0; tmp < SQUARE_COUNT; ++tmp) {
        *dest = *src;
        ++dest;
        ++src;
        if((tmp & 7) == 7) {
            dest += 120;
            src += 120;
        }
    }
}

static char subwindow_rect_test() {
    if(!my(window_open)) return 0;

    if(u(mouse_display_x - (my(window_x)+1)) > u(my(window_w)-4)) return 0;
    if(u(mouse_display_y - (my(window_y)+12)) > u(my(window_h)-13)) return 0;

    return 1;
}

static void mines_app_handler(char e) {
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite(my(window_x), my(window_y), mines_app_W, mines_app_H, 0, 0, my(window_sprite));
            break;
        case WINDOW_EVENT_TICK:
            break;
        case WINDOW_EVENT_LATE_TICK:
            if(ctx->square_clicked != 255) {
                if(ctx->square_clicked & WAS_RIGHT_CLICK) {
                    ctx->square_clicked &= 63;
                    if((ctx->squares[ctx->square_clicked] & SQUARE_REVEALED) == 0) {
                        tmp = 10;
                        if((ctx->squares[ctx->square_clicked] & SQUARE_FLAGGED)) tmp = 13;
                        direct_prepare_sprite_ram_array_mode(my(window_sprite));
                        copy8x8(addr_for_tile(tmp), addr_for_square(ctx->square_clicked));
                        ctx->squares[ctx->square_clicked] ^= SQUARE_FLAGGED;
                    }
                } else {
                    if((ctx->squares[ctx->square_clicked] & SQUARE_REVEALED) == 0) {
                        ctx->square_clicked &= 63;
                        direct_prepare_sprite_ram_array_mode(my(window_sprite));
                        copy8x8(addr_for_tile(ctx->squares[ctx->square_clicked] & 0xF), addr_for_square(ctx->square_clicked));
                        if(ctx->squares[ctx->square_clicked] != MINE_SQUARE) {
                            ctx->click_count--;
                        } else {
                             copy8x8(addr_for_tile(14), vram+412);
                             ctx->game_done = 1;
                        }
                        ctx->squares[ctx->square_clicked] |= SQUARE_REVEALED;

                        if(ctx->click_count == 0) {
                            copy8x8(addr_for_tile(12), vram+412);
                            ctx->game_done = 1;
                        }
                        
                    }
                }
                 
                ctx->square_clicked = 255;
            }
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
        case WINDOW_EVENT_RIGHT_CLICK:
            if(ctx->game_done) break;
            if(subwindow_rect_test()) {
                ctx->square_clicked = (((mouse_display_x - my(window_x)) - 1) >> 3)
                    + (((mouse_display_y - my(window_y)) - 12) & 0x38);
                if(e == WINDOW_EVENT_RIGHT_CLICK)
                    ctx->square_clicked |= WAS_RIGHT_CLICK;
            }
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
            break;
        case WINDOW_EVENT_EXIT:
            free_sprite(my(window_sprite));
            mem_free(my(window_context));
            --mines_app_open_count;
            break;
        default:
            break;
    }
}

void mines_app_launch() {
    if(desktop_launch_app(mines_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(mines_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__mines_bmp_load_list);
        my(window_x) = 32 + (mines_app_open_count<<2);
        my(window_y) = 24 + (mines_app_open_count<<2);
        my(window_w) = mines_app_W;
        my(window_h) = mines_app_H;

        ctx->click_count = 64;
        ctx->game_done = 0;

        for(tmp = 0; tmp < SQUARE_COUNT; ++tmp) {
            ctx->squares[tmp] = 0;
        }
        for(tmp = 0; tmp < MINE_COUNT; ++tmp) {
            tmp2 = rnd_range(0, 63);
            if(ctx->squares[tmp2] < 9) {
                ctx->click_count--;
                ctx->squares[tmp2] = MINE_SQUARE;
                if((tmp2 & 7)) ctx->squares[tmp2-1]++;
                if((tmp2 & 7) < 7) ctx->squares[tmp2+1]++;
                if(tmp2 > 7) {
                    ctx->squares[tmp2-8]++;
                    if((tmp2 & 7)) ctx->squares[tmp2-9]++;
                    if((tmp2 & 7) < 7) ctx->squares[tmp2-7]++;
                }
                if(tmp2 < 56) {
                    ctx->squares[tmp2+8]++;
                    if((tmp2 & 7)) ctx->squares[tmp2+7]++;
                     if((tmp2 & 7) < 7) ctx->squares[tmp2+9]++;
                }
            }
        }
        for(tmp = 0; tmp < SQUARE_COUNT; ++tmp) {
            if(ctx->squares[tmp] > MINE_SQUARE) {
                ctx->squares[tmp] = MINE_SQUARE;
            }
        }
        ctx->square_clicked = 255;
        
        ++mines_app_open_count;
    }
}