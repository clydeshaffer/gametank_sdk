#ifndef DRAWING_H
#define DRAWING_H

#include "vga.h"
#include "geom.h"

#define INSIDE 0
#define LEFT 1
#define RIGHT 2
#define BOTTOM 4
#define TOP 8

#define WALL 1
#define CEILING 2
#define FLOOR 4

typedef int Outcode;

typedef struct viewport
{
    int top, bottom, left, right;
} viewport;


extern byte *gradient_buffer;
void highlight_portal(viewport portal, byte color);
bool portal_overlap(viewport *sub, viewport *super);
bool clip_line_to_viewport(point2 *pointA, point2 *pointB, viewport portal);
byte sample_gradient(int x, int y, int t);
byte depth_to_shade(coord_t depth);
void vert(int xpos, int ystart, int ylen, byte color, int gradient_mode);
void rect(int xpos, int ypos, int width, int height, byte color);
void grad_rect_h(int xpos, int ypos, int width, int height);
void grad_rect_v(int xpos, int ypos, int width, int height);
void draw_line2(int ax, int ay, int bx, int by, byte color);
void draw_line_screen(point2 a, point2 b, byte color);
void line_3d(point3 a, point3 b, byte color, point3 camerap, byte cnt);
void interpolate_buffer(int ax, int ay, int bx, int by, byte *target_array);
void make_gradient(int ax, int ay, int bx, int by, byte *target_array);
void draw_wall_screen(int ax, int ay, int bx, int by, int gradient_type, byte color);
void draw_wall(point2 pointA, point2 pointB, coord_t floor, coord_t ceiling, viewport portal, int flags, int base_color, int wallmask);
void check_and_draw_wall(point2 a, point2 b, coord_t floor, coord_t ceiling, gap *neighbor_heights, byte color, point3 camerap, byte cnt, viewport portal);

#endif