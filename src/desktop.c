#include "desktop.h"
#include "mouse.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/feature/random/random.h"
#include "util.h"
#include "gen/assets/gfx.h"

#include "apps/draw.h"
#include "apps/spikeball.h"
#include "apps/mines.h"


#define MAX_ICONS 6
char icons_x[MAX_ICONS] = { 24, 64, 72, 32, 80, 90};
char icons_y[MAX_ICONS] = { 24, 64, 36, 72, 90, 64};
char icons_f[MAX_ICONS] = {  3,  1,  2,  1,  4,  5};
char icons_app[MAX_ICONS] = {0, 0, 2, 0, 1, 3};
char dragging_index = 255;
char dragged_app = 0;
char drag_rel_x = 0;
char drag_rel_y = 0;

SpriteSlot bg_sprite;
SpriteSlot icons_sprite;

char app_to_launch = 0;
char current_app = 0;
char app_draw_index = 0;
char window_open[MAX_APPS] = {0};
char window_draw_order[MAX_APPS];
char window_x[MAX_APPS];
char window_y[MAX_APPS];
char window_w[MAX_APPS];
char window_h[MAX_APPS];
void* window_context[MAX_APPS];
char frames_since_click = 255;
char last_window_clicked = MAX_APPS;
SpriteSlot window_sprite[MAX_APPS];
char window_to_bump = 0xFF;
char window_to_close = 0xFF;

void (*window_handler[MAX_APPS]) (char);
static char tmp = 0;

void remove_from_draw_order(char appslot) {
    char hit = 0xFF;
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == appslot) {
            hit = app_draw_index;
            break;
        }
    }
    if(hit == 0xFF) return;
    for(app_draw_index = hit; app_draw_index < MAX_APPS-1; ++app_draw_index) {
        window_draw_order[app_draw_index] = window_draw_order[app_draw_index+1];
    }
    window_draw_order[MAX_APPS-1] = 0xFF;
}

void bump_to_top(char appslot) {
    remove_from_draw_order(appslot);
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) {
            window_draw_order[app_draw_index] = appslot;
            return;
        }
    }
}

char window_rect_test(char wi) {
    if(!window_open[wi]) return 0;
    if(u(mouse_display_x - window_x[wi]) > window_w[wi]) return 0;
    if(u(mouse_display_y - window_y[wi]) > window_h[wi]) return 0;
    return 1;
}

void desktop_init() {
    bg_sprite = allocate_sprite(&ASSET__gfx__desktop_bmp_load_list);
    icons_sprite = allocate_sprite(&ASSET__gfx__icons_bmp_load_list);
    set_sprite_frametable(icons_sprite, ASSET__gfx__icons_json);
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        window_draw_order[app_draw_index] = 0xFF;
    }
}

char find_window_under_cursor() {

}

char desktop_launch_app(void(*handler)(char)) {
    for(current_app = 0; current_app < MAX_APPS; ++current_app) {
        if(!window_open[current_app]) {
            window_open[current_app] = 1;
            window_handler[current_app] = handler;
            for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
                if(window_draw_order[app_draw_index] == 0xFF) {
                    window_draw_order[app_draw_index] = current_app;
                    return current_app;
                }
            }
            //should not be possible to get past this inner for loop
        }
    }
    return 255;
}

void desktop_update() {
    rnd();
    if(mouseStatus & (~oldMouseStatus) & 3) {
        //Iterate backwards from draw order
        for(app_draw_index = MAX_APPS-1; app_draw_index != 255; --app_draw_index) {
            if(window_draw_order[app_draw_index] == 0xFF) continue;
            current_app = window_draw_order[app_draw_index];
            if(window_rect_test(current_app)) {
                if(u(mouse_display_y - window_y[current_app]) < 4) {
                    if(u(mouse_display_x - window_x[current_app]) < 3) {
                        window_handler[current_app](WINDOW_EVENT_EXIT);
                        window_open[current_app] = 0;
                        window_to_close = current_app;
                    } else {
                        dragging_index = 'w';
                        dragged_app = current_app;
                        drag_rel_x = window_x[current_app] - mouse_display_x;
                        drag_rel_y = window_y[current_app] - mouse_display_y;
                        window_to_bump = current_app;
                    }
                } else {
                    window_to_bump = current_app;
                    last_window_clicked = current_app;
                    if(mouseStatus & (~oldMouseStatus) & 1)
                        window_handler[current_app](WINDOW_EVENT_MOUSE_CLICK);
                    if(mouseStatus & (~oldMouseStatus) & 2)
                        window_handler[current_app](WINDOW_EVENT_RIGHT_CLICK);
                }
                break;
            }
        }

        if(app_draw_index == 255) {
            for(tmp = MAX_ICONS-1; tmp != 0xFF; --tmp) {
                if(icons_f[tmp]) {
                    if(cabs(icons_x[tmp] - mouse_display_x) < 8) {
                        if(cabs(icons_y[tmp] - mouse_display_y) < 8) {
                            if(icons_app[tmp] && (frames_since_click < 15)) {
                                app_to_launch = icons_app[tmp];
                                break;
                            } else {
                                dragging_index = tmp;
                                drag_rel_x = icons_x[tmp] - mouse_display_x;
                                drag_rel_y = icons_y[tmp] - mouse_display_y;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if(window_to_close != 0xFF) {
            remove_from_draw_order(window_to_close);
            window_to_close = 0xFF;
        }

        if(window_to_bump != 0xFF) {
            bump_to_top(window_to_bump);
            window_to_bump = 0xFF;
        }

        frames_since_click = 0;
    } else if((~mouseStatus) & oldMouseStatus & 1) {
        if(last_window_clicked < MAX_APPS) {
            window_handler[last_window_clicked](WINDOW_EVENT_MOUSE_RELEASE);
            last_window_clicked = MAX_APPS;
        } else if((dragging_index != 0) && (dragging_index < MAX_ICONS)) {
                if(cabs(icons_x[dragging_index] - icons_x[0]) < 8) {
                if(cabs(icons_y[dragging_index] - icons_y[0]) < 8) {
                    icons_f[dragging_index] = 0;
                }
            }   
        }
        dragging_index = 255;
    }
    
    if(dragging_index < MAX_ICONS) {
        icons_x[dragging_index] = drag_rel_x + mouse_display_x;
        icons_y[dragging_index] = drag_rel_y + mouse_display_y;
    } else if(dragging_index == 'w') {
        window_x[dragged_app] = drag_rel_x + mouse_display_x;
        window_y[dragged_app] = drag_rel_y + mouse_display_y;
    }

    for(current_app = 0; current_app < MAX_APPS; ++current_app) {
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_TICK);
        }
    }
    if(frames_since_click < 255) ++frames_since_click;
}

void desktop_early_draw() {
    queue_draw_sprite(0,0,127,127,0,0,bg_sprite);
}

void desktop_draw() {
    for(tmp = 0; tmp < MAX_ICONS; ++tmp) {
        if(icons_f[tmp]) {
            queue_draw_sprite_frame(icons_sprite, icons_x[tmp], icons_y[tmp], icons_f[tmp], 0);
        }
    }

    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) continue;
        current_app = window_draw_order[app_draw_index];
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_DRAW);
        }
    }

    queue_draw_sprite_frame(icons_sprite, mouse_display_x, mouse_display_y, 0, 0);
    queue_clear_border(0);
    await_draw_queue();
}

void desktop_late_update() {
    for(current_app = 0; current_app < MAX_APPS; ++current_app) {
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_LATE_TICK);
        }
    }

    switch(app_to_launch) {
        case 0: break;
        case 1: draw_app_launch(); break;
        case 2: spike_app_launch(); break;
        case 3: mines_app_launch(); break;
    }
    app_to_launch = 0;
}