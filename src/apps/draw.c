#include "gametank.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../mouse.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"

typedef struct {
char draw_color;
} draw_app_context;

#define ctx ((draw_app_context*)my(window_context))

char sample_pixel = 0xFF;
char drag_in_window = 0xFF;
char draw_app_open_count = 0;

#define DRAW_APP_W 66
#define DRAW_APP_H 75

static char subwindow_rect_test() {
    if(!my(window_open)) return 0;

    if(u(mouse_display_x - (my(window_x)+1)) > u(my(window_w)-2)) return 0;
    if(u(mouse_display_y - (my(window_y)+4)) > u(my(window_h)-11)) return 0;

    return 1;
}

static void draw_app_handler(char e) {
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite(my(window_x), my(window_y), DRAW_APP_W, DRAW_APP_H, 0, 0, my(window_sprite));
            break;
        case WINDOW_EVENT_TICK:
            break;
        case WINDOW_EVENT_LATE_TICK:
            if(drag_in_window == current_app) {
                if(subwindow_rect_test()) {
                    direct_prepare_sprite_ram_array_mode(my(window_sprite));
                    vram[((mouse_display_y - my(window_y)) << 7) + ((mouse_display_x - my(window_x)) & 0x7F)] = ctx->draw_color;
                }
            }
            if(sample_pixel == current_app) {
                sample_pixel = 0xFF;
                direct_prepare_sprite_ram_array_mode(my(window_sprite));
                ctx->draw_color = vram[((mouse_display_y - my(window_y)) << 7) + ((mouse_display_x - my(window_x)) & 0x7F)];
            }
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
            if((mouse_display_y - my(window_y) > 69)) {
                sample_pixel = current_app;
            } else {
                drag_in_window = current_app;
            }
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
                drag_in_window = 0xFF;
            break;
        case WINDOW_EVENT_EXIT:
                free_sprite(my(window_sprite));
                mem_free(my(window_context));
                --draw_app_open_count;
            break;
        default:
            break;
    }
}

void draw_app_launch() {
    if(desktop_launch_app(draw_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(draw_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__draw_bmp_load_list);
        my(window_x) = 32 + (draw_app_open_count<<2);
        my(window_y) = 24 + (draw_app_open_count<<2);
        my(window_w) = DRAW_APP_W;
        my(window_h) = DRAW_APP_H;
        ctx->draw_color = 32;
        ++draw_app_open_count;
    }
}