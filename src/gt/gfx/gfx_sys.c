/*
 * GameTank-specific implementation of drawing functions
 */
#include "gfx_sys.h"
#include "gametank.h"
#include "banking.h"

char cursorX, cursorY;
char draw_busy;

void await_vsync(int frames) {
    int i;
    for(i = 0; i < frames; i++) {
        frameflag = 1;
        while(frameflag) {}
    }
}

void flip_pages() {
    frameflip ^= DMA_PAGE_OUT;
    bankflip ^= BANK_SECOND_FRAMEBUFFER;
    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ | DMA_OPAQUE | frameflip | DMA_GCARRY;
    *dma_flags = flagsMirror;
    *bank_reg = bankflip;
}

void init_graphics() {
    frameflip = 0;
    draw_busy = 0;
    flagsMirror = DMA_NMI | DMA_ENABLE | DMA_IRQ;
    bankflip = BANK_SECOND_FRAMEBUFFER;
    *dma_flags = flagsMirror;
    banksMirror = bankflip;
    *bank_reg = banksMirror;
}

void await_drawing() {
    asm ("CLI");
    while (draw_busy)
    {
        wait();
    }
}
