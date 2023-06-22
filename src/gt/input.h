#ifndef INPUT_H
#define INPUT_H

extern int player1_buttons, player1_old_buttons;
extern int player2_buttons, player2_old_buttons;

#define INPUT_MASK_UP		2056
#define INPUT_MASK_DOWN		1028
#define INPUT_MASK_LEFT		512
#define INPUT_MASK_RIGHT	256
#define INPUT_MASK_A		16
#define INPUT_MASK_B		4096
#define INPUT_MASK_C		8192
#define INPUT_MASK_START	32
#define INPUT_MASK_ALL_KEYS INPUT_MASK_UP|INPUT_MASK_DOWN|INPUT_MASK_LEFT|INPUT_MASK_RIGHT|INPUT_MASK_A|INPUT_MASK_B|INPUT_MASK_C|INPUT_MASK_START

void update_inputs();

#endif