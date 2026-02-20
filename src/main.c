#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/sprites.h"
#include "gen/assets/reimu.h"
#include "gen/assets/reimu2.h"
#include "gen/assets/marisa.h"
#include "gen/assets/marisa2.h"
#include "gt/input.h"
char box_x = 30, box_y = 20;
char dx = 1, dy = 1;

SpriteSlot reimu_odd_rows;
SpriteSlot reimu_even_rows;

SpriteSlot marisa_odd_rows;
SpriteSlot marisa_even_rows;

char scanline_flips = 0;

char scan_ticks = 0xE1;
char scan_ticks_h = 0;
char start_flip = DMA_PAGE_OUT;
char start_delay = 18;
char delay_i = 0;

char art_select = 0;

void main () {

    reimu_odd_rows = allocate_sprite(&ASSET__reimu__reimu_odd_rows_bmp_load_list);
    reimu_even_rows = allocate_sprite(&ASSET__reimu2__reimu_even_rows_bmp_load_list);

    marisa_odd_rows = allocate_sprite(&ASSET__marisa__marisa_odd_bmp_load_list);
    marisa_even_rows = allocate_sprite(&ASSET__marisa2__marisa_even_bmp_load_list);

    queue_draw_sprite(0, 0, 127, 127, 0, 0, reimu_odd_rows);
    //queue_clear_screen(7);
    queue_clear_border(32);
    await_draw_queue();
    flip_pages();
    queue_draw_sprite(0, 0, 127, 127, 0, 0, reimu_even_rows);
    //queue_clear_screen(32);
    queue_clear_border(32);
    await_draw_queue();

    scanline_flips = 1;

    via[ACR] = 0b11000000;

    while (1) {                                     //  Run forever
        await_vsync(1);
        via[IER] = 0b01000000;
        update_inputs();
        if(player1_new_buttons & INPUT_MASK_UP) {
            ++scan_ticks;
            if(scan_ticks == 0) ++scan_ticks_h;
        }
        if(player1_new_buttons & INPUT_MASK_DOWN) {
            if(scan_ticks == 0) -- scan_ticks_h;
            --scan_ticks;
        }
        if(player1_new_buttons & INPUT_MASK_A) start_flip = start_flip ^ DMA_PAGE_OUT;
        if(player1_new_buttons & INPUT_MASK_LEFT) --start_delay;
        if(player1_new_buttons & INPUT_MASK_RIGHT) ++start_delay;
        if(player1_new_buttons & INPUT_MASK_START) {
            ++art_select;
            scanline_flips = 0;
            banksMirror &= ~BANK_SECOND_FRAMEBUFFER;
            *bank_reg = banksMirror;
            flagsMirror |= DMA_IRQ;
            *dma_flags = flagsMirror;
            if(art_select & 1) { 
                queue_draw_sprite(0, 0, 127, 127, 0, 0, marisa_even_rows);
                queue_clear_border(32);
                await_draw_queue();
                flip_pages();
                queue_draw_sprite(0, 0, 127, 127, 0, 0, marisa_odd_rows);
                queue_clear_border(32);
                await_draw_queue();
            } else {
                queue_draw_sprite(0, 0, 127, 127, 0, 0, reimu_odd_rows);
                queue_clear_border(32);
                await_draw_queue();
                flip_pages();
                queue_draw_sprite(0, 0, 127, 127, 0, 0, reimu_even_rows);
                queue_clear_border(32);
                await_draw_queue();
            }
            scanline_flips = 1;
        }
        frameflip = start_flip;
        flagsMirror = DMA_NMI | DMA_ENABLE | frameflip;
        *dma_flags = flagsMirror;
        delay_i = start_delay;
        while(--delay_i) {}
        
        via[T1LL] = scan_ticks;
        via[T1CH] = scan_ticks_h;
        via[IER] = 0b11000000;
    }
}