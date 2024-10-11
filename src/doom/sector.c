#include <stdlib.h>
#include <stdio.h>
#include "sector.h"

int world_sector_count;
Sector* world_sector_list;
int current_sector = 0;
int world_vertex_count;
point2* world_vertex_list;

bool point_inside_sector(point2 point, Sector s) {
    int i;
    coord_t side = line_side(point, world_vertex_list[s.corners[0]], world_vertex_list[s.corners[1]]);
    int sign = side < 0;
    for(i = 1; i < s.corner_count; i++) {
        side = line_side(point, world_vertex_list[s.corners[i]], world_vertex_list[s.corners[(i+1) % s.corner_count]]);
        if (sign != (side < 0)) return false;
    }
    return true;
}

void free_first_n_sectors_members(int n) {
    int i;
    for(i = 0; i < n; i ++) {
        free(world_sector_list[i].corners);
        free(world_sector_list[i].neighbors);
    }
}

int load_sectors_from_file(char* filename) {
    FILE *fp;
    char buf[1024];
    int i,k, scanned;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("coult not open file %s\n", filename);
        return 0;
    }

    if(!fgets(buf,1024, fp)) {
        printf("file %s was empty\n", filename);
        return 0;
    }

    if(sscanf(buf, "%d %d", &world_vertex_count, &world_sector_count) != 2 ) {
        printf("header of file %s was invalid\n", filename);
        return 0;
    }

    world_vertex_list = malloc(sizeof(point2) * world_vertex_count);

    for(i = 0; i < world_vertex_count; i++) {
        if(!fgets(buf,1024, fp)) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            printf("unexpected end of file in vertex section\n");
            return 0;
        }
        if(sscanf(buf, " %ld %ld", &(world_vertex_list[i].x), &(world_vertex_list[i].y)) != 2 ) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            printf("invalid format for vertex\n");
            return 0;
        }
    }

    world_sector_list = malloc(sizeof(Sector) * world_sector_count);

    for(i = 0; i < world_sector_count; i++) {
        if(!fgets(buf,1024, fp)) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            free_first_n_sectors_members(i);
            free(world_sector_list);
            world_sector_list = NULL;
            printf("unexpected end of file in sector %d\n", i);
            return 0;
        }
        if(sscanf(buf, "%d", &(world_sector_list[i].corner_count)) != 1 ) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            free_first_n_sectors_members(i);
            free(world_sector_list);
            world_sector_list = NULL;
            printf("invalid format for corner count in sector %d\n", i);
            return 0;
        }
        world_sector_list[i].color_offset = i;
        world_sector_list[i].ceiling_color = (i % 8) + 21;
        world_sector_list[i].ground_color = (i % 8) + 20;
        world_sector_list[i].corners = malloc(sizeof(int) * world_sector_list[i].corner_count);
        world_sector_list[i].neighbors = malloc(sizeof(int) * world_sector_list[i].corner_count);
        for(k = 0; k < world_sector_list[i].corner_count; k++) {
            if(!fgets(buf,1024, fp)) {
                free(world_vertex_list);
                world_vertex_list = NULL;
                free(world_sector_list);
                free_first_n_sectors_members(i+1);
                world_sector_list = NULL;
                printf("unexpected end of file in sector section\n");
                return 0;
            }
            scanned = sscanf(buf, " %d %d", &(world_sector_list[i].corners[k]), &(world_sector_list[i].neighbors[k]));
            if(scanned < 1) {
                free(world_vertex_list);
                world_vertex_list = NULL;
                free(world_sector_list);
                free_first_n_sectors_members(i+1);
                world_sector_list = NULL;
                printf("invalid format for corner reference in sector %d, corner %d\n", i, k);
                return 0;
            }
            if(scanned == 1) {
                world_sector_list[i].neighbors[k] = -1;
            }
        }
        if(!fgets(buf,1024, fp)) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            free(world_sector_list);
            free_first_n_sectors_members(i+1);
            world_sector_list = NULL;
            printf("unexpected end of file at floor/ceiling heights in sector %d\n", i);
            return 0;
        }
        printf("%d\n", world_sector_list[i].corner_count);
        if(sscanf(buf, " %ld %ld", &(world_sector_list[i].floor), &(world_sector_list[i].ceiling)) != 2 ) {
            free(world_vertex_list);
            world_vertex_list = NULL;
            free_first_n_sectors_members(i);
            free(world_sector_list);
            world_sector_list = NULL;
            printf("invalid format for floor/ceiling heights in sector %d\n", i);
            return 0;
        }
    }
    return 1;
}