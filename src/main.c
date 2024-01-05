#include "gametank.h"
#include "drawing_funcs.h"
#include "gen/assets/art.h"
#include <zlib.h>
#include "gt/banking.h"

int main () {

    init_graphics();

    *dma_flags = DMA_CPU_TO_VRAM;
    *bank_reg = 0;

    change_rom_bank(ASSET__art__BeeshSpweeshNewYear_bmp_bank);
    inflatemem(vram, &ASSET__art__BeeshSpweeshNewYear_bmp_ptr);

    while (1) {                                     //  Run forever
        sleep(1);
    }

  return (0);                                     //  We should never get here!
}