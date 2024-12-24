#include "./gametank.h"
#include "./gfx/draw_direct.h"
#include "./audio/music.h"
#include "../gen/bank_nums.h"
#include "../gen/assets/sdk_default.h"

#pragma code-name (push, "LOADERS")
#pragma rodata-name (push, "LOADERS")

//ASM defined in draw_logo.s
void draw_gametank_logo(char color);
static const char logo_colors[] = {0, 0, 25, 57, 90, 122, 155, 187, 220, 252};
static const char fadeout_colors[] = { 7, 6, 5, 4, 3, 2, 1, 32};
static char logo_state;

void sdk_init() {
    init_graphics();
    logo_state = 0;
    while(++logo_state) {
        direct_prepare_box_mode();
        if(logo_state < 3) {
            DIRECT_DRAW_COLOR(127, 127, 1, 1, 8);
            DIRECT_DRAW_COLOR(0, 0, 127, 127, 8);
            if(logo_state == 2) {
                init_audio_coprocessor();
                init_music();
            }
            await_drawing();
            DIRECT_DRAW_COLOR(127, 0, 1, 127, 8);
            await_drawing();
            DIRECT_DRAW_COLOR(0, 127, 127, 1, 8);
            await_drawing();
        }
        
        if(logo_state < 120 + (sizeof(fadeout_colors)<<2)) {
            if(logo_state > 120) {
                draw_gametank_logo(~fadeout_colors[(logo_state - 120) >> 2]);
            } else if(logo_state < 40) {
                draw_gametank_logo(~logo_colors[logo_state >> 2]);
            } else {
                draw_gametank_logo(~7);
            }
        }
        flip_pages();
        await_vsync(1);
        if(logo_state == 3) {
            play_song(&ASSET__sdk_default__jingle_mid, REPEAT_NONE);
        }
        if(logo_state > 2) {
            tick_music();
        }
        if(logo_state == 180) logo_state = 255;
    }
    stop_music();
}

#pragma code-name (pop)
#pragma rodata-name (pop)