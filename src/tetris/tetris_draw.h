#ifndef TETRIS_DRAW_H

#include "tetris_logic.h"

#define TETRIS_DRAW_H


void init_tetromino_minis();

void printnum(int num);

void print(char* str);

void draw_field0(char x, char y);
void draw_field1(char x, char y);

void draw_piece(PiecePos* pos, const char* piece, char offsetX, char offsetY);

void draw_mini(const char tet_index, char x, char y);

void drawPlayerState(PlayerState* player);

void printnum_bcd(int num);

extern char cursorX, cursorY;

extern const unsigned char tetro_colors[TET_COUNT+2];

#endif