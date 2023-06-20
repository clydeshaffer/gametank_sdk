#include "gametank.h"

char frameflag = 0;

/* Set framebuffer 0 to output to screen */
char frameflip = 0;

/* Set framebuffer 1 as DMA target */
char bankflip = BANK_SECOND_FRAMEBUFFER;

char flagsMirror = 0;
char banksMirror = 0;