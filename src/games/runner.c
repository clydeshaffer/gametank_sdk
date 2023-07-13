#include "gametank.h"
#include "drawing_funcs.h"
#include "input.h"
#include "./common.h"
#include "feature/text/text.h"
#include "../gen/assets/gfx3.h"
#include "../gen/assets/gfx4.h"
#include "../gen/assets/music.h"
#include "random.h"
#include "music.h"

#define PLAYER_X 24
#define GROUND_Y 116
#define GRAVITY 15
#define FAST_GRAVITY 25
#define JUMP_VEL -512
#define START_SCROLL_RATE 192
static coordinate player_y;
static coordinate player_vy;
static coordinate anim_frame;
static coordinate bg_scroll;
static coordinate scroll_rate;
static char i,k;
static char anim_state;
static char slide_timer;

#define ANIM_STATE_STAND 0
#define ANIM_STATE_RUN 1
#define ANIM_STATE_JUMP 2
#define ANIM_STATE_SLIDE 3
#define ANIM_STATE_HURT 4

static const char anim_offsets[5] = { 0, 1, 25, 39, 43 };

#define ANIM_OFFSET_STAND 0
#define ANIM_OFFSET_RUN 1
#define ANIM_OFFSET_JUMP 25
#define ANIM_OFFSET_SLIDE 39
#define ANIM_OFFSET_HURT 43

static const char anim_lengths[5] = { 1, 24, 14, 4, 6 };

#define ANIM_LENGTH_STAND 1
#define ANIM_LENGTH_RUN 24
#define ANIM_LENGTH_JUMP 14
#define ANIM_LENGTH_SLIDE 4
#define ANIM_LENGTH_HURT 6


static const char anim_rates[5] = { 128, 128, 48, 64, 32};

#define GAME_STATE_TITLE 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_GAMEOVER 2

static coordinate spike_x;
static char spike_y;

const char start_lives_text[] = "LIVES: |||";
const char start_score_text[] = "SCORE: ";
char lives_text[11];
char score_text[32];

void init_texts() {
    for(i = 0; i < sizeof(start_lives_text); ++i) {
        lives_text[i] = start_lives_text[i];        
    }
    for(i = 0; i < sizeof(start_score_text); ++i) {
        score_text[i] = start_score_text[i];
    }
}

void update_texts() {
    lives_text[lives + 7] = 0;
    if(score) {
        i = score;
        k = 1;
        while(i > 0) {
            score_text[32-k] = '0' + (i % 10);
            i /= 10;
            ++k;
        }
        i = 0;
        while(k > 0) {
            score_text[6+k] = score_text[32-i];
            --k;
            ++i;
        }
    } else {
        score_text[7] = '0';
        score_text[8] = 0;
    }
}

void player_physic() {
    player_y.i += player_vy.i;
    if(player_y.b.msb < GROUND_Y) {
        if((anim_frame.b.msb == (ANIM_LENGTH_HURT-1)) && (anim_frame.b.lsb & 128)) {
            anim_state = ANIM_STATE_JUMP;
            anim_frame.b.msb = ANIM_LENGTH_JUMP >> 1;
        }
        if(player1_buttons & (INPUT_MASK_A | INPUT_MASK_UP))
            player_vy.i += GRAVITY;
        else
            player_vy.i += FAST_GRAVITY;
    } else {
        player_y.b.msb = GROUND_Y;
        player_y.b.lsb = 0;
        player_y.b.msb = GROUND_Y;
        player_y.b.lsb = 0;
        if(anim_state == ANIM_STATE_HURT) {
            if((anim_frame.b.msb == (ANIM_LENGTH_HURT-1)) && (anim_frame.b.lsb & 128)) {
                anim_state = ANIM_STATE_RUN;
                anim_frame.i = 0;
            }
        } else if(anim_state == ANIM_STATE_SLIDE) {
            if(slide_timer) {
                --slide_timer;
            } else {
                anim_state = ANIM_STATE_RUN;
            }
        } else if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_A | INPUT_MASK_UP)) {
            player_vy.i = JUMP_VEL;
            player_y.i += player_vy.i;
            anim_state = ANIM_STATE_JUMP;
            anim_frame.i = 0;
            do_noise_effect(48, 64, 10);
        } else if(player1_buttons & ~player1_old_buttons & (INPUT_MASK_B | INPUT_MASK_DOWN)) {
            anim_state = ANIM_STATE_SLIDE;
            anim_frame.i = 0;
            slide_timer = 60;
            do_noise_effect(72, -4, 15);
        } else {
            anim_state = ANIM_STATE_RUN;
        }
    }
}

static char check_collision() {
    if(delta(PLAYER_X, spike_x.b.msb) > 8) return 0;
    if(player_y.b.msb < spike_y) {
        //player feet above spikeball center        
        return (spike_y - player_y.b.msb) < 8;
    } else {
        //player feet below spikeball center
        if(anim_state == ANIM_STATE_SLIDE) {
            return spike_y > (GROUND_Y - 26);
        } else {
            return spike_y > (GROUND_Y - 46);
        }
    }
}

void run_runner_game() {
    await_draw_queue();
    sleep(1);
    flip_pages();
    
    load_big_spritesheet(&ASSET__gfx3__redhood, 0);
    load_spritesheet(&ASSET__gfx3__spikeball_bmp, 1);
    load_wide_spritesheet(&ASSET__gfx4__forest, 2);
    rnd_seed = 234;
    global_tick = 0;

    anim_state = 0;
    player_y.b.lsb = 0;
    player_y.b.msb = 64;
    anim_frame.i = 0;
    spike_y = GROUND_Y - 8;
    spike_x.b.msb = -64;
    spike_x.b.lsb = 0;
    scroll_rate.b.msb = START_SCROLL_RATE;
    scroll_rate.b.lsb = 0;
    score = 0;
    lives = 3;
    play_song(&ASSET__music__badapple_mid, REPEAT_LOOP);
    init_texts();
    while(1) {
        update_inputs();
        bg_scroll.i += scroll_rate.b.msb;
        if(scroll_rate.b.msb < START_SCROLL_RATE) {
            --spike_x.b.msb;
            ++bg_scroll.b.msb;
        }

        draw_sprite(0, 0, 127, 127, bg_scroll.b.msb, 0, 2);

        spike_x.i -= scroll_rate.b.msb;
        if((spike_x.b.msb > 180) && (spike_x.b.msb < 200)) {
            spike_y = GROUND_Y - 8 - ((rnd_range(0, 3)) * 10);
            spike_x.b.msb = 128+32;
            ++score;
            update_texts();
        }

        anim_frame.i += anim_rates[anim_state];
        ++anim_frame.i;
        anim_frame.b.msb = anim_frame.b.msb % anim_lengths[anim_state];

        
        player_physic();

        if(anim_state != ANIM_STATE_HURT) {
            if(check_collision()) {
                if(anim_state == ANIM_STATE_JUMP) {
                    player_vy.i = -player_vy.i;
                }
                anim_state = ANIM_STATE_HURT;
                --lives;
                anim_frame.i = 0;
                do_noise_effect(100, -64, 8);
                update_texts();
            }
        }

        draw_sprite_frame(&ASSET__gfx3__spikeball_json, spike_x.b.msb, spike_y, (global_tick >> 2) & 15, 0, 1 | BANK_CLIP_X);
        draw_sprite_frame(&ASSET__gfx3__redhood_json, PLAYER_X, player_y.b.msb, anim_offsets[anim_state] + anim_frame.b.msb, SPRITE_FLIP_X, 0);
        
        await_draw_queue();
        clear_border(0);
        await_draw_queue();

        text_print_width = 128;
        text_print_line_start = 4;
        text_cursor_x = 4;
        text_cursor_y = 8;
        text_use_alt_color = 1;
        print_text(lives_text);
        print_text("\r\n");
        print_text(score_text);

        sleep(1);
        flip_pages();
        ++global_tick;
        ++scroll_rate.i;
        tick_music();
    }
}