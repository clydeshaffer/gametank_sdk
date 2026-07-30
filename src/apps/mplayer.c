#include "gametank.h"
#include "../gt/audio/music.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../mouse.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"
#include "../gt/feature/random/random.h"
#include "../gen/assets/music.h"
#include "../gen/assets/music2.h"
#pragma code-name (push, "PROG0")

#define TRACK_COUNT 8

#define TITLE_CORNER_ADDR vram+513

typedef struct {
    char track_number;
    char loop_mode;
    char change_title;
    char music_playing;
} mplayer_app_context;

#define ctx ((mplayer_app_context*)my(window_context))

static char tmp, tmp2;
char mplayer_app_open_count = 0;

#define mplayer_app_W 66
#define mplayer_app_H 24

static char* addr_for_title(char title) {
    return vram + 8192 + (title << 10);
}

static void copyTitle(char* src, char* dest) {
    for(tmp2 = 0; tmp2 < 8; ++tmp2) {
        for(tmp = 0; tmp < 64; ++tmp) {
            *dest = *src;
            ++dest;
            ++src;
        }
        dest += 64;
        src += 64;
    }
}

static char subwindow_rect_test() {
    if(!my(window_open)) return 0;

    if(u(mouse_display_x - (my(window_x)+1)) > u(my(window_w)-4)) return 0;
    if(u(mouse_display_y - (my(window_y)+15)) > u(my(window_h)-16)) return 0;

    return 1;
}

static void play_track_by_num(char num) {
    switch(num) {
        case 0:
            play_song(&ASSET__music2__badapple_mid, ctx->loop_mode);
            break;
        case 1:
            play_song(&ASSET__music__groove_mid, ctx->loop_mode);
            break;
        case 2:
            play_song(&ASSET__music__vampire_mid, ctx->loop_mode);
            break;
        case 3:
            play_song(&ASSET__music__brinstar_mid, ctx->loop_mode);
            break;
        case 4:
            play_song(&ASSET__music__Jungle1_mid, ctx->loop_mode);
            break;
        case 5:
            play_song(&ASSET__music__labs1_mid, ctx->loop_mode);
            break;
        case 6:
            play_song(&ASSET__music__mines1_mid, ctx->loop_mode);
            break;
        case 7:
            play_song(&ASSET__music__burnV2_mid, ctx->loop_mode);
            break;
        default:
            break;
    }
}

static void mplayer_app_handler(char e) {
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite(my(window_x), my(window_y), mplayer_app_W, mplayer_app_H, 0, 0, my(window_sprite));
            break;
        case WINDOW_EVENT_TICK:
            tick_music();
            break;
        case WINDOW_EVENT_LATE_TICK:
            if(ctx->change_title != 0xFF) {
                ctx->change_title = 0xFF;
                direct_prepare_sprite_ram_array_mode(my(window_sprite));
                copyTitle(addr_for_title(ctx->track_number), TITLE_CORNER_ADDR);
            }
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
            if(subwindow_rect_test()) {
                tmp = (mouse_display_x - (my(window_x) + 1)) >> 4;
                switch(tmp) {
                    case 0:
                        ctx->track_number--;
                        if(ctx->track_number == 255) ctx->track_number = TRACK_COUNT-1;
                        if(ctx->music_playing) {
                            play_track_by_num(ctx->track_number);
                        }
                        ctx->change_title = 1;
                        break;
                    case 1:
                        if(!ctx->music_playing) {
                            ctx->music_playing = 1;
                            play_track_by_num(ctx->track_number);
                        }
                        break;
                    case 2:
                        ctx->track_number++;
                        if(ctx->track_number == TRACK_COUNT) ctx->track_number = 0;
                        if(ctx->music_playing) {
                            play_track_by_num(ctx->track_number);
                        }
                        ctx->change_title = 1;
                        break;
                    case 3: 
                        if(ctx->music_playing) stop_music();
                        ctx->music_playing = 0;
                        break;
                    default:
                        break;
                }
            }
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
            break;
        case WINDOW_EVENT_EXIT:
            free_sprite(my(window_sprite));
            mem_free(my(window_context));
            --mplayer_app_open_count;
            break;
        default:
            break;
    }
}

void mplayer_app_launch() {
    if(desktop_launch_app(mplayer_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(mplayer_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__mplayer_bmp_load_list);
        my(window_x) = 32 + (mplayer_app_open_count<<2);
        my(window_y) = 24 + (mplayer_app_open_count<<2);
        my(window_w) = mplayer_app_W;
        my(window_h) = mplayer_app_H;

        ctx->track_number = 0;
        ctx->loop_mode = REPEAT_LOOP;
        ctx->change_title = 0xFF;
        ctx->music_playing = 1;
        
        ++mplayer_app_open_count;
        init_music();
        play_song(&ASSET__music2__badapple_mid, REPEAT_LOOP);
    }
}