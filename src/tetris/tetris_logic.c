/* 
 * Game logic implementation
 *
 * Mostly decoupled from the GameTank platform.
 * gametank.h is imported for the input button masks
 */
#include "tetris_logic.h"
#include "../gt/gametank.h"
#include "../gt/input.h"
#include "../gt/random.h"
#include "../gt/music.h"
#include "../gen/assets/music.h"

#pragma code-name(push, "PROG0")

PlayerState players[2];
char playField_0[FIELD_W*FIELD_H];
char playField_1[FIELD_W*FIELD_H];
char tmpPieceBuf[PIECEBUF_SIZE];

const unsigned char rotation_matrix[PIECEBUF_SIZE] = {
    20,15,10, 5, 0,
    21,16,11, 6, 1,
    22,17,12, 7, 2,
    23,18,13, 8, 3,
    24,19,14, 9, 4
};

const unsigned char tetro_index[TET_COUNT] = {0, PIECEBUF_SIZE, PIECEBUF_SIZE*2, PIECEBUF_SIZE*3, PIECEBUF_SIZE*4, PIECEBUF_SIZE*5, PIECEBUF_SIZE*6};

/*
 * Tetrominoes in I J L O S T Z order
 * Values represent the color, 0=clear
 */
const unsigned char tetrominoes[PIECEBUF_SIZE*TET_COUNT] = {
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,
      0,  1,  1,  1,  1,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  2,  0,  0,  0,
      0,  2,  2,  2,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  0,  0,  3,  0,
      0,  3,  3,  3,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  0,  4,  4,  0,
      0,  0,  4,  4,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  0,  5,  5,  0,
      0,  5,  5,  0,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  0,  6,  0,  0,
      0,  6,  6,  6,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,
      0,  7,  7,  0,  0,
      0,  0,  7,  7,  0,
      0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,
};

/* Convert half-byte kick offsets to actual signed chars */
char k2o[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, -7, -6, -5, -4, -3, -2, -1};

/* Every shape except I and O */
char kicks_main[20] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x1F, 0x02, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0xFF, 0x02, 0xF2,
};

char kicks_I[20] = {
    0x00, 0xF0, 0x20, 0xF0, 0x20,
    0xF0, 0x00, 0x00, 0x01, 0x0E,
    0xF1, 0x11, 0xE1, 0x10, 0xE0,
    0x01, 0x01, 0x01, 0x0F, 0x02,
};

void init_piece(char type, PiecePos* pos, char* dest) {
    char r, c, i;
    type &= 0x7F;
    type = type % 7;
    i = tetro_index[type];
    pos->x = SPAWN_COL;
    pos->y = SPAWN_ROW;
    pos->t = type;
    pos->rot = 0;
    pos->lock = 0;
    for(r = 0; r < PIECEBUF_WIDTH; r++) {
        for(c = 0; c < PIECEBUF_WIDTH; c++) {
            *dest = tetrominoes[i];
            dest++;
            i++;
        }
    }
}

void place_at(PiecePos* pos, char* pieceBuf, char* field) {
    char r, c, i = 0, j;
    j = (pos->y * FIELD_W) + pos->x;
    j -= (FIELD_W*2 + 2);
    for(r = 0; r < PIECEBUF_WIDTH; r++) {
        for(c = 0; c < PIECEBUF_WIDTH; c++) {
            if(pieceBuf[i]) {
                field[j] = pieceBuf[i];
            }
            i++;
            j++;
        }
        j += 5;
    }
}

int test_at(PiecePos* pos, char* pieceBuf, char* field) {
    char r, c, i = 0, j;
    j = (pos->y * FIELD_W) + pos->x;
    j -= (FIELD_W*2) + 2;
    for(r = 0; r < PIECEBUF_WIDTH; r++) {
        for(c = 0; c < PIECEBUF_WIDTH; c++) {
            if(pieceBuf[i]) {
                if((pos->x + c - 2) >= FIELD_W) {
                    return 0;
                } else if((pos->x + c - 2) == -1) {
                    return 0;
                }  else if((pos->y + r - 2) >= FIELD_H) {
                    return 0;
                } else if(field[j]) {
                    return 0;
                }
            }
            i++;
            j++;
        }
        j += FIELD_W - PIECEBUF_WIDTH;
    }
    return 1;
}

int hard_drop(PiecePos* pos, char* pbp, char* ffp) {
    static char pc, pr, fc, fr;
    static char *pieceCursor;
    static char finalY;
    static char* pieceBuf, *field;
    pieceBuf = pbp;
    field = ffp;
    finalY = FIELD_H;
    for(pc = 0; pc < PIECEBUF_WIDTH; pc++) {
        pieceCursor = pieceBuf + PIECEBUF_WIDTH * (PIECEBUF_WIDTH-1) + pc;
        fc = pc + pos->x - 2;
        for(pr = PIECEBUF_WIDTH; pr > 0; pr--) {
            if(*pieceCursor) {
                break;
            } else {
                pieceCursor -= PIECEBUF_WIDTH;
            }
        }
        if(pr > 0) {
            fr = pr + pos->y - 3;
            pieceCursor = field + (fr * FIELD_W) + fc;
            for(; fr < FIELD_H; fr++) {
                if(*pieceCursor) {
                    break;
                } else {
                    pieceCursor += FIELD_W;
                }
            }
            fr = fr - pr + 2;
            if(fr < finalY) {
                finalY = fr;
            }
        }
    }

    pos->lock = LOCK_FRAMES+1;
    pos->y = finalY+1;
    return pos->y-1;
}

void copyPiece(char* src, char* dest) {
    char r = 0;
    for(r = 0; r < PIECEBUF_SIZE; r++) {
        *dest = *src;
        dest++; src++;
    }
}

void rotateRight(char* pieceBuf) {
    char i = 0;

    copyPiece(pieceBuf, tmpPieceBuf);

    for(i=0;i<PIECEBUF_SIZE;i++) {
        *pieceBuf = tmpPieceBuf[rotation_matrix[i]];
        pieceBuf++;
    }
}

void rotateLeft(char* pieceBuf) {
    char i = 0;

    copyPiece(pieceBuf, tmpPieceBuf);

    for(i=0;i<PIECEBUF_SIZE;i++) {
        pieceBuf[rotation_matrix[i]] = tmpPieceBuf[i];
    }
}

void tryRotate(PiecePos* posp, char* pieceBuf, char* field, char direction) {
    static char newRot;
    static char oldX, oldY, i;
    static char* kicksrc, *kickdst;
    static PiecePos *pos;
    pos = posp;
    newRot = (pos->rot + direction + 4) % 4;
    oldX = pos->x;
    oldY = pos->y;
    
    if(pos->t == TET_O) {
        return;
    }
    if(direction == 1) {
        rotateRight(pieceBuf);
    } else {
        rotateLeft(pieceBuf);
    }
    if(pos->t == TET_I) {
        kicksrc = kicks_I + (NUM_KICKTESTS * pos->rot);
        kickdst = kicks_I + (NUM_KICKTESTS * newRot);
    } else {
        kicksrc = kicks_main + (NUM_KICKTESTS * pos->rot);
        kickdst = kicks_main + (NUM_KICKTESTS * newRot);
    }

    for(i = 0; i < NUM_KICKTESTS; i ++) {
        pos->x += k2o[(kicksrc[i] >> 4) & 15];
        pos->y -= k2o[kicksrc[i] & 15];
        pos->x -= k2o[(kickdst[i] >> 4) & 15];
        pos->y += k2o[kickdst[i] & 15];
        if(1 == test_at(pos, pieceBuf, field)) {
            pos->rot = newRot;
            return;
        }
        pos->x = oldX;
        pos->y = oldY;
    }


    if(direction == 1) {
        rotateLeft(pieceBuf);
    } else {
        rotateRight(pieceBuf);
    }
}

#define T_SPIN_NONE 0
#define T_SPIN_MINI 1
#define T_SPIN_FULL 2
const int cornerOffsets[5] = {
    0 - FIELD_W - 1,
    0 - FIELD_W + 1,
    0 + FIELD_W + 1,
    0 + FIELD_W - 1,
    0 - FIELD_W - 1
};

char checkTSpin0(PiecePos* pos) {
    static char center, count, x, y, rot;
    center = (pos->y * FIELD_W) + pos->x;
    count = 0;
    x = pos->x; y = pos->y; rot = pos->rot;
    count += !!playField_0[center - FIELD_W - 1] || (x == 0);
    count += !!playField_0[center - FIELD_W + 1] || (x == FIELD_W-1);
    count += !!playField_0[center + FIELD_W - 1] || (x == 0) || (y == FIELD_H-1);
    count += !!playField_0[center + FIELD_W + 1] || (x == FIELD_W-1) || (y == FIELD_H-1);
    if(count > 2) {
        count = 0;
        count += !!playField_0[center + cornerOffsets[rot]];
        count += !!playField_0[center + cornerOffsets[rot+1]];

        if(count == 2) {
            return T_SPIN_FULL;
        } else {
            return T_SPIN_MINI;
        }
    } else {
        return T_SPIN_NONE;
    }
}

char checkTSpin1(PiecePos* pos) {
    static char center, count, x, y, rot;
    center = (pos->y * FIELD_W) + pos->x;
    count = 0;
    x = pos->x; y = pos->y; rot = pos->rot;
    count += !!playField_1[center - FIELD_W - 1] || (x == 0);
    count += !!playField_1[center - FIELD_W + 1] || (x == FIELD_W-1);
    count += !!playField_1[center + FIELD_W - 1] || (x == 0) || (y == FIELD_H-1);
    count += !!playField_1[center + FIELD_W + 1] || (x == FIELD_W-1) || (y == FIELD_H-1);
    if(count > 2) {
        count = 0;
        count += !!playField_1[center + cornerOffsets[rot]];
        count += !!playField_1[center + cornerOffsets[rot+1]];

        if(count == 2) {
            return T_SPIN_FULL;
        } else {
            return T_SPIN_MINI;
        }
    } else {
        return T_SPIN_NONE;
    }
}

char checkLineClears0(char topBound, char botBound) {
    static char r, c, i, j, clearCount, blocks;
    i = (FIELD_W*(botBound+1)) - 1;
    clearCount = 0;
    blocks = 0;
    for(r = botBound; r >= topBound; r--) {
        blocks = 0;
        for(c = 0; c < FIELD_W; c++) {
            blocks += !!playField_0[i--];
        }
        if(blocks == FIELD_W) {
            clearCount++;
            r++;
            i+=FIELD_W;
            for(j = i; j >= FIELD_W; j--) {
                playField_0[j] = playField_0[j-FIELD_W];
            }
        }
    }
    return clearCount;
}

char checkLineClears1(char topBound, char botBound) {
    static char r, c, i, j, clearCount, blocks;
    i = (FIELD_W*(botBound+1)) - 1;
    clearCount = 0;
    blocks = 0;
    for(r = botBound; r >= topBound; r--) {
        blocks = 0;
        for(c = 0; c < FIELD_W; c++) {
            blocks += !!playField_1[i--];
        }
        if(blocks == FIELD_W) {
            clearCount++;
            r++;
            i+=FIELD_W;
            for(j = i; j >= FIELD_W; j--) {
                playField_1[j] = playField_1[j-FIELD_W];
            }
        }
    }
    return clearCount;
}

char garbageTable[10] = {0, 0, 1, 2, 4, 0, 2, 4, 6, 0};
char comboGarbage[10] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5};

void addGarbage(char* playField, char amount) {
    char i, len, nlen;
    char *fieldD = playField, *fieldS;
    if(amount > FIELD_H) {
        amount = FIELD_H;
    }
    len = FIELD_W * amount;
    nlen = FIELD_W * (FIELD_H - amount);
    fieldS = &(playField[len]);
    for(i = 0; i < nlen; i ++) {
        *fieldD = *fieldS;
        fieldD++;
        fieldS++;
    }
    for(i = nlen; i < FIELD_H * FIELD_W; i++) {
        *fieldD = 8;
        fieldD++;
    }
    for(i = nlen; i < FIELD_H * FIELD_W; i+=FIELD_W) {
        playField[i + ((rnd() & 127) % FIELD_W)] = 0;
    }
}

void init_bag(char *fullbag) {
    char i;
    for(i = 0; i < TET_COUNT*2; i++) {
        fullbag[i] = i % TET_COUNT;
    }
}

void shuffle_bag(char *bag) {
    char tmp, i, pick;
    for(i = 0; i < TET_COUNT; i++) {
        pick = ((rnd() & 0x7F) % (TET_COUNT - i)) + i;
        tmp = bag[i];
        bag[i] = bag[pick];
        bag[pick] = tmp;
    }
}

char take_next_piece(PlayerState* player) {
    char next_piece = player->bag[player->bag_index];
    player->bag_index = (player->bag_index + 1) % (TET_COUNT*2);
    player->bag_anim = 8;
    if(player->bag_index == 0) {
        shuffle_bag(player->bag+TET_COUNT);
    } else if(player->bag_index == TET_COUNT) {
        shuffle_bag(player->bag);
    }
    return next_piece;
}

void initPlayerState(PlayerState* player) {
    init_bag(player->bag);
    shuffle_bag(player->bag);
    shuffle_bag(player->bag+TET_COUNT);
    init_piece(player->bag[0], &(player->currentPos), player->currentPiece);
    player->bag_index = 1;
    player->bag_anim = 0;
    player->fallRate = 10;
    player->fallTimer = 0;
    player->score = 0;
    player->heldPiece.rot = 0;
    player->heldPiece.t = TET_COUNT;
    player->flags = 0;
    player->pendingGarbage = 0;
    player->combo = 0;
    player->heldPiece.x = 0;
    player->heldPiece.y = 0;
}

char updatePlayerState(PlayerState* p, int inp, int last_inp) {
    static char oldX, oldY, tmp, tmp2, tSpinType, garbageOut;
    static int tmpscore;
    static int inputs, last_inputs;
    static PlayerState* player;
    inputs = inp;
    last_inputs = last_inp;
    player = p;
    if(~last_inputs & inputs & (INPUT_MASK_LEFT | INPUT_MASK_RIGHT)) {
        player->movetime = nmi_count;
    }
    garbageOut = 0;
    if(player->flags & PLAYER_DEAD) {
        return 0;
    }
    oldX = player->currentPos.x;
        oldY = player->currentPos.y;
        if(player->fallTimer < player->fallRate){
            player->currentPos.y++;
        } else if(!(player->flags&PLAYER_DIDHOLD) && (inputs & INPUT_MASK_C & ~last_inputs)) {
            if(player->heldPiece.t == TET_COUNT) {
                tmp = take_next_piece(player);
            } else {
                tmp = player->heldPiece.t;
            }
            player->heldPiece.t = player->currentPos.t;
            init_piece(tmp,  &(player->currentPos), player->currentPiece);
            player->fallTimer = 255 - player->fallRate;
            player->flags |= PLAYER_DIDHOLD;
        } else if(inputs & INPUT_MASK_UP & ~last_inputs) {
            oldY = hard_drop(&(player->currentPos), player->currentPiece, player->playField);
        } else if(inputs & INPUT_MASK_A & ~last_inputs) {
            tryRotate(&(player->currentPos), player->currentPiece, player->playField, -1);
        } else if(inputs & INPUT_MASK_B & ~last_inputs) {
            tryRotate(&(player->currentPos),player-> currentPiece, player->playField, 1);
        } else {
            if(inputs & INPUT_MASK_LEFT) {
                if(((nmi_count - player->movetime) & 0x3) == 0)
                    player->currentPos.x--;
            }
            if(inputs & INPUT_MASK_RIGHT) {
                if(((nmi_count - player->movetime) & 0x3) == 0)
                    player->currentPos.x++;
            }
            if(inputs & INPUT_MASK_DOWN) {
            player->currentPos.y++;
            }
        }

        if(0 == test_at(&(player->currentPos), player->currentPiece, player->playField)){
            if(player->currentPos.x == oldX && player->currentPos.y == oldY) {
                player->flags |= PLAYER_DEAD;
            } else if(player->currentPos.y > oldY) {
                tmp = player->currentPos.x;
                player->currentPos.x = oldX;
                if(0 == test_at(&(player->currentPos), player->currentPiece, player->playField)){
                    player->currentPos.y = oldY;
                    player->currentPos.x = tmp;
                    if(tmp != oldX) {
                        if(0 == test_at(&(player->currentPos), player->currentPiece, player->playField)) {
                            player->currentPos.x = oldX;
                        } else {
                            player->currentPos.lock = 0;
                        }
                    }

                    if(player->currentPos.lock > LOCK_FRAMES) {
                        tSpinType = 0;
                        if(player->currentPos.t == TET_T) {
                            if(player->playernum)
                                tSpinType = checkTSpin1(&(player->currentPos));
                            else
                                tSpinType = checkTSpin0(&(player->currentPos));

                        }

                        place_at(&(player->currentPos), player->currentPiece, player->playField);
                        tmp = player->currentPos.y;
                        tmp2 = tmp;
                        if(tmp < 3) {
                            tmp = 1;
                        } else {
                            tmp -= 2;
                        }
                        if(tmp2 > FIELD_H-3) {
                            tmp2 = FIELD_H-1;
                        } else {
                            tmp2 += 2;
                        }
                        if(player->playernum) {
                            tmp = checkLineClears1(tmp, tmp2);
                        } else {
                            tmp = checkLineClears0(tmp, tmp2);
                        }
                        tmpscore = player->score;

                        asm("SED");
                        tmpscore += tmp;
                        asm("CLD");

                        player->score = tmpscore;

                        garbageOut = garbageTable[tmp + (4 * (tSpinType == T_SPIN_FULL))];

                        if(tSpinType == T_SPIN_FULL) {
                            play_sound_effect(&ASSET__music__tspin_bin, 5);
                        } else if(tmp == 4) {
                            play_sound_effect(&ASSET__music__4clear_bin, 4);
                        } else if(tmp) {
                            play_sound_effect(&ASSET__music__1clear_bin, 2);
                        }

                        if(tmp != 0) {
                            if((tmp == 4) || tSpinType == T_SPIN_FULL) {
                                if(player->flags & PLAYER_BACK_TO_BACK) {
                                    garbageOut++;
                                } else {
                                    player->flags |= PLAYER_BACK_TO_BACK;
                                }
                            } else {
                                player->flags &= ~PLAYER_BACK_TO_BACK;
                            }
                            if(player->combo > 9) {
                                garbageOut += comboGarbage[9];
                            } else {
                                garbageOut += comboGarbage[player->combo];
                            }
                            player->combo++;
                        } else {
                            player->combo = 0;
                        }

                        if(player->pendingGarbage > garbageOut) {
                            player->pendingGarbage -= garbageOut;
                            garbageOut = 0;
                        } else {
                            garbageOut -= player->pendingGarbage;
                            player->pendingGarbage = 0;
                        }
                        
                        if((tmp == 0) && (player->pendingGarbage != 0)) {
                            addGarbage(player->playField, player->pendingGarbage);
                            player->pendingGarbage = 0;
                        }

                        init_piece(take_next_piece(player),  &(player->currentPos), player->currentPiece);
                        player->fallTimer = 255 - player->fallRate;
                        player->flags &= ~PLAYER_DIDHOLD;
                    } else {
                        if((inputs & ~INPUT_MASK_DOWN) == 0) {
                            player->currentPos.lock+=10;
                        } else {
                            player->currentPos.lock++;
                        }
                    }
                }
            } else {
                player->currentPos.y = oldY;
                player->currentPos.x = oldX;   
            }
        }
        player->fallTimer+=player->fallRate;
        return garbageOut;
}
#pragma code-name(pop)