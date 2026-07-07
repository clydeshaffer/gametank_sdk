// This module provides an alternative to the standard update_inputs() function.
// If ENABLE_MODULE_PADDLEINPUT is active, use update_paddle_inputs()
// to capture extended data for paddles and touch interfaces.

#ifndef PADDLEINPUT_H
#define PADDLEINPUT_H

#include "../../input.h"
#include "../../../gen/modules_enabled.h"

#ifndef ENABLE_MODULE_PADDLEINPUT
#error "Module PADDLEINPUT included but not enabled!"
#endif

#ifdef ENABLE_MODULE_PADDLEINPUT

#define INPUT_MASK_PADDLE_UP		2048    //prioritizes high, was 2056
#define INPUT_MASK_PADDLE_DOWN		1024    //prioritizes high, was 1028 
#define INPUT_MASK_PADDLE_LEFT		512     //high
#define INPUT_MASK_PADDLE_RIGHT	256     //high
#define INPUT_MASK_PADDLE_A		16      //low
#define INPUT_MASK_PADDLE_B		4096    //high
#define INPUT_MASK_PADDLE_C		8192    //high
#define INPUT_MASK_PADDLE_START	32      //low
#define INPUT_MASK_PADDLE_MODE 1           //low
#define INPUT_MASK_PADDLE_X 2              //low
#define INPUT_MASK_PADDLE_Y 4              //low
#define INPUT_MASK_PADDLE_Z 8              //low
#define INPUT_MASK_PADDLE_ALL_KEYS (INPUT_MASK_PADDLE_UP|INPUT_MASK_PADDLE_DOWN|INPUT_MASK_PADDLE_LEFT|INPUT_MASK_PADDLE_RIGHT|INPUT_MASK_PADDLE_A|INPUT_MASK_PADDLE_B|INPUT_MASK_PADDLE_C|INPUT_MASK_PADDLE_START|INPUT_MASK_PADDLE_MODE|INPUT_MASK_PADDLE_X|INPUT_MASK_PADDLE_Y|INPUT_MASK_PADDLE_Z)//add new keys

void update_paddle_inputs();
unsigned char get_paddle_rotation(char);

#endif //ifdef ENABLE_MODULE_PADDLEINPUT
#endif //ifdef PADDLEINPUT_H
