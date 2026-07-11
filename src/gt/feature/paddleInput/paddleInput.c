#include "../../input.h"
#include "paddleInput.h"
#include "../../../gen/modules_enabled.h"
#include "../../gametank.h"

#ifdef ENABLE_MODULE_PADDLEINPUT

#pragma optimize (push, off)
void update_paddle_inputs(){
    char inputsA, inputsB;
    inputsA = *gamepad_2;
    inputsA = *gamepad_1;
    inputsB = *gamepad_1;

    player1_old_buttons = player1_buttons;
    player1_buttons = ~((((int) inputsB) << 8) | inputsA);
    player1_buttons &= INPUT_MASK_PADDLE_ALL_KEYS;
    player1_new_buttons = player1_buttons & ~player1_old_buttons;

    inputsA = *gamepad_2;
    inputsB = *gamepad_2;
    player2_old_buttons = player2_buttons;
    player2_buttons = ~((((int) inputsB) << 8) | inputsA);
    player2_buttons &= INPUT_MASK_PADDLE_ALL_KEYS;
    player2_new_buttons = player2_buttons & ~player2_old_buttons;
}
#pragma optimize (pop)

unsigned char get_paddle_rotation(char port) {
    unsigned char result = 0;
    int _player_buttons;

    if (port==1) _player_buttons = player2_buttons;
    else _player_buttons = player1_buttons;//fallback to player 1 at port 0

    if (_player_buttons & INPUT_MASK_PADDLE_UP) result |= (1 << 0);
    if (_player_buttons & INPUT_MASK_PADDLE_DOWN) result |= (1 << 1);
    if (_player_buttons & INPUT_MASK_PADDLE_LEFT) result |= (1 << 2);
    if (_player_buttons & INPUT_MASK_PADDLE_RIGHT) result |= (1 << 3);
    if (_player_buttons & INPUT_MASK_PADDLE_X) result |= (1 << 4);
    if (_player_buttons & INPUT_MASK_PADDLE_Y) result |= (1 << 5);
    if (_player_buttons & INPUT_MASK_PADDLE_Z) result |= (1 << 6);
    if (_player_buttons & INPUT_MASK_PADDLE_MODE) result |= (1 << 7);

    return ~result;//invert result
}

#endif