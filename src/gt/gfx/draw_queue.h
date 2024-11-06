#ifndef DRAW_QUEUE_H
#define DRAW_QUEUE_H
#include "gfx_sys.h"
#include "sprites.h"

#define QUEUE_MAX 250

extern Frame rect; //Defined in draw_util.s
extern unsigned char queue_start, queue_end, queue_pending, queue_count;
extern unsigned char queue_flags_param; //defined in draw_util.s

void queue_draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c);
void queue_draw_sprite_rect();

#define queue_draw_sprite(X,Y,W,H,GX,GY,SPRITESLOT) rect.x=X;rect.y=Y;rect.w=W;rect.h=H;rect.gx=GX;rect.gy=GY;rect.b=SPRITESLOT;queue_draw_sprite_rect();
void queue_draw_sprite_frame(SpriteSlot sprite, char x, char y, char frame, char flip);


void queue_clear_border(char c);
void queue_clear_screen(char c);

//defined in draw_util.s
void next_draw_queue();

void await_draw_queue();

#endif