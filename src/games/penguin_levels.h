#ifndef PENGUIN_LEVELS_H
#define PENGUIN_LEVELS_H
#define MAZE_OFFSET_X_PIX 4
#define MAZE_OFFSET_ROW 4

#define HALF_LEVEL_WIDTH 7
#define HALF_LEVEL_HEIGHT 10
#define HALF_LEVEL_COUNT 16
#define HALF_LEVEL_BYTES HALF_LEVEL_HEIGHT*HALF_LEVEL_COUNT

extern const char half_levels[HALF_LEVEL_BYTES];

void load_half_level(char level_num, char side, char tilenum);

void load_level_num();

#endif