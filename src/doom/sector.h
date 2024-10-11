#ifndef SECTOR_H
#define SECTOR_H

#include "geom.h"

typedef struct Sector
{
    coord_t floor, ceiling;
    int corner_count;
    int* corners;
    int* neighbors;
    int color_offset;
    byte ground_color;
    byte ceiling_color;
} Sector;

extern int world_sector_count;
extern Sector* world_sector_list;
extern int current_sector;
extern int world_vertex_count;
extern point2* world_vertex_list;

bool point_inside_sector(point2 point, Sector s);

#endif