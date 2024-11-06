#ifndef GFX_SYS_H

#define GFX_SYS_H

#define CLIP_MODE_NONE 0
#define CLIP_MODE_X 16
#define CLIP_MODE_Y 32
#define CLIP_MODE_XY 48

#define SPRITE_FLIP_NONE 0
#define SPRITE_FLIP_X 1
#define SPRITE_FLIP_Y 2
#define SPRITE_FLIP_BOTH 3

#define QUADRANT_0 0
#define QUADRANT_1 8
#define QUADRANT_2 16
#define QUADRANT_3 24

#define BG_COLOR 16
#define WINDOW_COLOR 0

#define BORDER_BOTTOM_HEIGHT 8
#define BORDER_TOP_HEIGHT 7
#define BORDER_LEFT_WIDTH 1
#define BORDER_RIGHT_WIDTH 1

typedef struct Frame {
    char x, y, w, h, gx, gy, c, b;
} Frame;

#define SET_RECT(xm,ym,wm,hm,gxm,gym,cm,bm) rect.x = xm;\
    rect.y = ym; \
    rect.w = wm; \
    rect.h = hm; \
    rect.gx = gxm; \
    rect.gy = gym; \
    rect.c = cm; \
    rect.b = bm; \

void await_vsync(int frames);
void flip_pages();
void init_graphics();
void await_drawing();

extern char draw_busy;
extern Frame rect; //Defined in draw_util.s

void printnum(int num);

void print_hex_num(char num);

#endif
