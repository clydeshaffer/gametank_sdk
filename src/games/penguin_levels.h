#ifndef PENGUIN_LEVELS_H
#define PENGUIN_LEVELS_H
#define MAZE_OFFSET_X_PIX 4
#define MAZE_OFFSET_ROW 4

#define HALF_LEVEL_WIDTH 7
#define HALF_LEVEL_HEIGHT 10
#define HALF_LEVEL_COUNT 1
#define HALF_LEVEL_BYTES HALF_LEVEL_HEIGHT*HALF_LEVEL_COUNT

#define TILE_WEB 162
#define TILE_HEART 128

void load_level_num();

#endif