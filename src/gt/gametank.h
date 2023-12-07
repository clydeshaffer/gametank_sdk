/*
 * GameTank-specific defines
 */

#ifndef GAMETANK_H
#define GAMETANK_H

#define audio_reset ((volatile char *) 0x2000)
#define audio_nmi ((volatile char *) 0x2001)
#define audio_rate ((volatile char *) 0x2006)
#define dma_flags ((volatile char *) 0x2007)
#define bank_reg ((volatile char *) 0x2005)
#define via ((volatile char*) 0x2800)
#define aram ((volatile char *) 0x3000)
#define vram ((volatile char *) 0x4000)
#define vram_VX ((volatile char *) 0x4000)
#define vram_VY ((volatile char *) 0x4001)
#define vram_GX ((volatile char *) 0x4002)
#define vram_GY ((volatile char *) 0x4003)
#define vram_WIDTH ((volatile char *) 0x4004)
#define vram_HEIGHT ((volatile char *) 0x4005)
#define vram_COLOR ((volatile char *) 0x4007)
#define vram_START ((volatile char *) 0x4006)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define DMA_ENABLE 1
#define DMA_PAGE_OUT 2
#define DMA_NMI 4
#define DMA_COLORFILL_ENABLE 8
#define DMA_GCARRY 16
#define DMA_CPU_TO_VRAM 32
#define DMA_IRQ 64
#define DMA_OPAQUE 128

#define BANK_GRAM_MASK 7
#define BANK_SECOND_FRAMEBUFFER 8
#define BANK_CLIP_X 16
#define BANK_CLIP_Y 32
#define BANK_RAM_MASK 192

#define GRAM_PAGE(x) x

#define VX 0
#define VY 1
#define GX 2
#define GY 3
#define WIDTH 4
#define HEIGHT 5
#define START 6
#define COLOR 7

#define gamepad_1 ((volatile char *) 0x2008)
#define gamepad_2 ((volatile char *) 0x2009)

#define ORB 0
#define ORA 1
#define DDRB 2
#define DDRA 3
#define T1C 5
#define ACR 11
#define PCR 12
#define IFR 13
#define IER 14

extern char frameflag, frameflip, flagsMirror, banksMirror, bankflip;

void wait ();
void nop5();
void nop10();

#define PROFILER_START(x) via[ORB] = 0x80; via[ORB] = x;
#define PROFILER_END(x) via[ORB] = 0x80; via[ORB] = x | 0x40;

#endif