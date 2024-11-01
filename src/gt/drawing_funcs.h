#ifndef DRAWING_FUNCS_H

#define DRAWING_FUNCS_H

#define SPRITE_CHAR_W 8
#define SPRITE_CHAR_H 8
#define SPRITE_ROW_0_F 0x68
#define SPRITE_ROW_G_V 0x70
#define SPRITE_ROW_W_Z 0x78
#define SPRITE_CHAR_BLANK_X 0x70
#define SPRITE_CHAR_BLANK_Y 0x78

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

#define SET_RECT(xm,ym,wm,hm,gxm,gym,cm,bm) rect.x = xm;\
    rect.y = ym; \
    rect.w = wm; \
    rect.h = hm; \
    rect.gx = gxm; \
    rect.gy = gym; \
    rect.c = cm; \
    rect.b = bm; \

typedef struct Frame {
    char x, y, w, h, gx, gy, c, b;
} Frame;

void sleep(int frames);
void flip_pages();
void init_graphics();

void load_spritesheet(char* spriteData, char srcBank, char ramBank);
#define load_big_spritesheet(NAME,BANK) \
load_spritesheet(NAME##_bmp, BANK); \
load_spritesheet(NAME##_1_bmp, BANK|QUADRANT_1); \
load_spritesheet(NAME##_2_bmp, BANK|QUADRANT_2); \
load_spritesheet(NAME##_3_bmp, BANK|QUADRANT_3);

void clear_spritebank(char bank);
void draw_sprite_frame(const Frame *sprite_table, char sprite_table_bank, char x, char y, char frame, char flip, char bank);

void draw_sprite_rect();
#define draw_sprite(X,Y,W,H,GX,GY,RAMBANK) rect.x=X;rect.y=Y;rect.w=W;rect.h=H;rect.gx=GX;rect.gy=GY;rect.b=RAMBANK;draw_sprite_rect();

void draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c);

void next_draw_queue();
void await_draw_queue();
void await_drawing();

void clear_border(char c);
void clear_screen(char c);

extern char draw_busy;

void prepare_sprite_mode(char ramBank);
void prepare_box_mode();

#define draw_sprite_now( x,  y,  w,  h,  gx,  gy,  ramBank) \
    flagsMirror |= DMA_ENABLE | DMA_OPAQUE | DMA_IRQ; \
    flagsMirror &= ~(DMA_COLORFILL_ENABLE | DMA_OPAQUE); \
    *dma_flags = flagsMirror; \
    banksMirror &= BANK_RAM_MASK & ~BANK_SECOND_FRAMEBUFFER; \
    banksMirror |= frameflip | ramBank; \
    *bank_reg = banksMirror; \
    await_drawing(); \
    vram[VX] = x; \
    vram[VY] = y; \
    vram[GX] = gx; \
    vram[GY] = gy; \
    vram[WIDTH] = w; \
    vram[HEIGHT] = h; \
    draw_busy = 1; \
    vram[START] = 1;

#define draw_box_now(x, y, w, h, c) \
    flagsMirror |= DMA_ENABLE | DMA_OPAQUE | DMA_IRQ | DMA_COLORFILL_ENABLE | DMA_OPAQUE; \
    *dma_flags = flagsMirror; \
    banksMirror &= BANK_RAM_MASK & ~BANK_SECOND_FRAMEBUFFER; \
    banksMirror |= bankflip; \
    *bank_reg = banksMirror; \
    await_drawing(); \
    vram[VX] = x; \
    vram[VY] = y; \
    vram[GX] = 0; \
    vram[GY] = 0; \
    vram[WIDTH] = w; \
    vram[HEIGHT] = h; \
    vram[COLOR] = ~(c); \
    draw_busy = 1; \
    vram[START] = 1;

void draw_fade(unsigned char opacity);

void printnum(int num);

void print_hex_num(char num);

void print(char* str);

extern char cursorX, cursorY;
extern unsigned char queue_start, queue_end, queue_pending, queue_count;
extern unsigned char queue_flags_param; //defined in draw_util.s

extern Frame rect; //Defined in draw_util.s

#endif
