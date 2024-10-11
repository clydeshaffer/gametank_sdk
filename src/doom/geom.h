#ifndef GEOM_H
#define GEOM_H

#define true 1
#define false 0

typedef long coord_t;

typedef struct point2 {
    coord_t x, y;
} point2;

typedef struct point3 {
    coord_t x, y, z;
} point3;

typedef struct gap {
    coord_t top, bottom;
} gap;

typedef unsigned char bool;

point3 pnt_223(point2 p, coord_t y);
point2 rotate2(point2 p, unsigned char angle);
point3 rotate3(point3 p, unsigned char angle);
point2 pnt2_shift(point2 p, int shiftby);
point3 worldToLocal(point3 p, point3 camera_pos, unsigned char angle);
point2 localToScreen(point3 localized, int screenw, int screenh);
point2 pnt_sub(point2 a, point2 b);
coord_t pnt_scross(point2 a, point2 b);
coord_t pnt_dot(point2 a, point2 b);
point2 pnt_neg(point2 a);
point2 pnt_flat(point3 a);
bool intersect(point2 a, point2 b, point2 c, point2 d, point2 *output);
bool intersect_originray(point2 a, point2 b, point2 c2d, point2 *output);
bool point_in_view_cone(point3 a);
bool left_of_cone(point2 a);
bool view_cone(point3 *a, point3 *b);
coord_t line_side(point2 p, point2 a, point2 b);
#endif