#include "gt/gfx/sprites.h"

#define WINDOW_EVENT_TICK 0
#define WINDOW_EVENT_MOUSE_CLICK 1
#define WINDOW_EVENT_MOUSE_RELEASE 2
#define WINDOW_EVENT_DRAW 3
#define WINDOW_EVENT_LATE_TICK 4
#define WINDOW_EVENT_EXIT 5

#define MAX_APPS 5
extern char current_app;
extern char window_open[MAX_APPS];
extern char window_x[MAX_APPS];
extern char window_y[MAX_APPS];
extern char window_w[MAX_APPS];
extern char window_h[MAX_APPS];
extern SpriteSlot window_sprite[MAX_APPS];

#define my(prop) (prop[current_app])

void desktop_init();
void desktop_update();
void desktop_early_draw();
void desktop_draw();
void desktop_late_update();

char desktop_launch_app(void(*handler)(char));