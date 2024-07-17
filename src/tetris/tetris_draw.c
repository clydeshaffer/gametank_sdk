/*
 * GameTank-specific implementation of drawing functions
 */
#include <zlib.h>
#include "../gt/drawing_funcs.h"
#include "../gt/feature/text/text.h"
#include "tetris_draw.h"
#include "gametank.h"

extern const unsigned char* GameSprites;
extern const unsigned char* BGSprite;
extern const unsigned char* TitleSprite;

const unsigned char tetro_colors[TET_COUNT+2] = { 0 , 244, 219, 92, 61, 29, 155, 124, 5};

extern void wait();
extern void nop5();
extern void nop10();

void init_tetromino_minis() {
    char i, x, y, r, c;
    char *vram_ptr, *tet_ptr;
    flagsMirror = DMA_NMI | DMA_IRQ;
    *dma_flags = flagsMirror;
    vram_ptr = vram;
    tet_ptr = tetrominoes;
    for(i = 0; i < TET_COUNT; i++) {
        for(r = 0; r < PIECEBUF_WIDTH; r++) {
            for(c = 0; c < PIECEBUF_WIDTH; c++) {
                *vram_ptr = tetro_colors[*tet_ptr];
                vram_ptr++;
                tet_ptr++;
            }
            vram_ptr += SCREEN_WIDTH - PIECEBUF_WIDTH;
        }
        vram_ptr += SCREEN_WIDTH * 3;
    }
    flagsMirror |= DMA_ENABLE;
    *dma_flags = flagsMirror;
}

void printnum(int num) {
    vram[VX] = cursorX;
    vram[VY] = cursorY;
    vram[GY] = SPRITE_ROW_0_F - 8;
    vram[WIDTH] = SPRITE_CHAR_W;
    vram[HEIGHT] = SPRITE_CHAR_H;
    if(num == 0) {
        vram[GX] = 0;
        vram[START] = 1;
        wait();
    } else {
        while(num != 0) {
            vram[GX] = (num % 16) << 3;
            vram[START] = 1;
            wait();
            cursorX -= 8;
            num = num >> 4;
            vram[VX] = cursorX;
        }
    }
}

void print(char* str) {
    vram[WIDTH] = SPRITE_CHAR_W;
    vram[HEIGHT] = SPRITE_CHAR_H;
    while(*str != 0) {
        if(*str >= '0' && *str <= '9') {
            vram[GX] = (*str - '0') << 3;
            vram[GY] = SPRITE_ROW_0_F;
        } else if(*str >= 'a' && *str <= 'f') {
            vram[GX] = ((*str - 'a') << 3) + 0x50;
            vram[GY] = SPRITE_ROW_0_F;
        } else if(*str >= 'g' && *str <= 'v') {
            vram[GX] = (*str - 'g') << 3;
            vram[GY] = SPRITE_ROW_G_V;
        } else if(*str >= 'w' && *str <= 'z') {
            vram[GX] = (*str - 'w') << 3;
            vram[GY] = SPRITE_ROW_W_Z;
        } else {
            vram[GX] = SPRITE_CHAR_BLANK_X;
            vram[GY] = SPRITE_CHAR_BLANK_Y;
        }
        if(*str == '\n') {
            cursorX = 0;
            cursorY += 8;
        } else {
            vram[VX] = cursorX;
            vram[VY] = cursorY;
            vram[START] = 1;
            wait();
            cursorX += 8;
        }
        str++;
        if(cursorX >= 128) {
            cursorX = 0;
            cursorY += 8;
        }
        if(cursorY >= 128) {
            cursorX = 0;
            cursorY = 0;
        }
    }
}

void draw_field0(char x, char y) {
    static char r, c, vx, vy, f, stx, sty, fieldIdx;
    stx = x;
    fieldIdx = 0;
    vram[GY] = 64;
    vram[WIDTH] = GRID_SPACING;
    vram[HEIGHT] = GRID_SPACING;
    vy = y;
    for(r = 0; r < FIELD_H; r++) {
        vx = stx;
        vram[VY] = vy;
        for(c = 0; c < FIELD_W; c++) {
            f = playField_0[fieldIdx];
            if(f) {
                vram[GX] = f<<2;
                vram[VX] = vx;
                vram[START] = 1;
            }
            vx+=GRID_SPACING;
            ++fieldIdx;
        }
        vy+=GRID_SPACING;
    }
}

void draw_field1(char x, char y) {
    static char r, c, vx, vy, f, stx, sty, fieldIdx;
    stx = x;
    fieldIdx = 0;
    vram[GY] = 64;
    vram[WIDTH] = GRID_SPACING;
    vram[HEIGHT] = GRID_SPACING;
    vy = y;
    for(r = 0; r < FIELD_H; r++) {
        vx = stx;
        for(c = 0; c < FIELD_W; c++) {
            f = playField_1[fieldIdx];
            if(f) {
                vram[GX] = f<<2;
                vram[VX] = vx;
                vram[VY] = vy;
                vram[START] = 1;
            }
            vx+=GRID_SPACING;
            ++fieldIdx;
        }
        vy+=GRID_SPACING;
    }
}

void draw_piece(PiecePos* pos, const char* piece, char offsetX, char offsetY) {
    static char r, c, i, px, py;
    i = 0;
    px = GRID_SPACING*pos->x;
    py = GRID_SPACING*pos->y;
    vram[GY] = 64;
    vram[WIDTH] = GRID_SPACING;
    vram[HEIGHT] = GRID_SPACING;
    for(r = 0; r < PIECEBUF_WIDTH*GRID_SPACING; r+=GRID_SPACING) {
        for(c = 0; c < PIECEBUF_WIDTH*GRID_SPACING; c+=GRID_SPACING) {
            if(!!piece[i]) {
                vram[GX] = piece[i]*4;
                vram[VX] = px + c + offsetX - 2*GRID_SPACING;
                vram[VY] = py + r + offsetY - 2*GRID_SPACING;
                vram[START] = 1;
                //wait();
            }
            i++;
        }
    }
}

void draw_mini(const char tet_index, char x, char y) {
    vram[GX] = 0;
    vram[GY] = tet_index * 8;
    vram[WIDTH] = PIECEBUF_WIDTH;
    vram[HEIGHT] = PIECEBUF_WIDTH;
    vram[VX] = x;
    vram[VY] = y;
    vram[START] = 1;
    //wait();
}

void drawPlayerState(PlayerState* player) {
    static char i, j, k, m, n;
    static char *bag;
    via[ORB] = 0x80;
    via[ORB] = 0x01;
    if(player->playernum) {
        draw_field1(player->field_offset_x, player->field_offset_y);
    } else {
        draw_field0(player->field_offset_x, player->field_offset_y);
    }
    via[ORB] = 0x80;
    via[ORB] = 0x41;
    draw_piece(&(player->currentPos), player->currentPiece, player->field_offset_x, player->field_offset_y);

    if(player->heldPiece.t != TET_COUNT) {
        i = 0;
        j = 0;
        if(player->heldPiece.t == TET_I) {
            i = 2;
            j = 2;
        }
        else if(player->heldPiece.t == TET_O) {
            i = 2;
        }
        draw_piece(
            &(player->heldPiece),
            &(tetrominoes[tetro_index[player->heldPiece.t]]),
            player->field_offset_x - i + 28, player->field_offset_y - j - 9);
    }
    
    draw_sprite_now(player->field_offset_x-2, player->field_offset_y-2, GRID_SPACING * FIELD_W+4, 8, 64, 0, 0);
    wait();
    cursorX = player->field_offset_x + SPRITE_CHAR_W + 4;
    cursorY = player->field_offset_y - SPRITE_CHAR_H - 4;
    printnum(player->score);
    bag = player->bag;
    j = player->bag_index;
    m = player->field_offset_x + player->bag_anim;
    n = player->field_offset_y;
    for(i = 0; i < PREVIEW_COUNT; i++) {
        draw_mini(bag[j], m, n);
        if(++j == (TET_COUNT*2)) j = 0;
        m += 8;
    }
    if(player->bag_anim > 0) {
        player->bag_anim--;
    }
    draw_box_now(player->field_offset_x + (i * 8), player->field_offset_y, PIECEBUF_WIDTH, PIECEBUF_WIDTH, 3);
    wait();
    draw_sprite_now(player->field_offset_x + (i * 8), player->field_offset_y,PIECEBUF_WIDTH, PIECEBUF_WIDTH, 106, 2, 0);
    wait();
    if(player->pendingGarbage != 0) {
        draw_box_now(
            player->field_offset_x + GRID_SPACING * FIELD_W + 2,
            player->field_offset_y + GRID_SPACING * FIELD_H - (player->pendingGarbage << 2),
            2,
            player->pendingGarbage << 2,
            6
        );
    }

    if(player->flags & PLAYER_DEAD) {
        cursorX = player->field_offset_x + (GRID_SPACING * (FIELD_W/2 - 5));
        cursorY = player->field_offset_y + (GRID_SPACING * FIELD_H/2);
        print("game");
        cursorX = player->field_offset_x + (GRID_SPACING * ((FIELD_W/2) - 3));
        cursorY = player->field_offset_y + (GRID_SPACING * FIELD_H/2) + SPRITE_CHAR_H;
        print("over");
    }
}
