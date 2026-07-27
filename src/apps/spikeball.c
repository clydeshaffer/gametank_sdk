#include "gametank.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../mouse.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"

typedef struct {
char dx, dy;
} spike_app_context;

#define ctx ((spike_app_context*)my(window_context))

#define spike_app_W 8
#define spike_app_H 8

static char spike_app_open_count = 0;

static void spike_app_handler(char e) {
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite_frame(my(window_sprite), my(window_x), my(window_y), (my(window_x) >> 2) & 15, 0);
            break;
        case WINDOW_EVENT_TICK:
            my(window_x) += ctx->dx;
            my(window_y) += ctx->dy;
            if(my(window_x) == 0) ctx->dx = 1;
            if(my(window_x) == 127) ctx->dx = 255;
            if(my(window_y) == 0) ctx->dy = 1;
            if(my(window_y) == 127) ctx->dy = 255;
            break;
        case WINDOW_EVENT_LATE_TICK:
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
            break;
        case WINDOW_EVENT_EXIT:
                free_sprite(my(window_sprite));
                mem_free(my(window_context));
                --spike_app_open_count;
            break;
        default:
            break;
    }
}

void spike_app_launch() {
    if(desktop_launch_app(spike_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(spike_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__spikeball_bmp_load_list);
        set_sprite_frametable(my(window_sprite), ASSET__gfx__spikeball_json);
        my(window_x) = mouse_display_x;
        my(window_y) = mouse_display_y;
        my(window_w) = spike_app_W;
        my(window_h) = spike_app_H;
        ctx->dx = ((spike_app_open_count << 1) & 2) - 1;
        ctx->dy = (spike_app_open_count & 2) - 1;
        ++spike_app_open_count;
    }
}