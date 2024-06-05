#include "banking.h"
#include "gametank.h"
#include "drawing_funcs.h"
#include "persist.h"

#pragma code-name (push, "DATA")
#pragma optimize (push, on)
char i, k;
void clear_save_sector() {
    *dma_flags = flagsMirror & ~(DMA_IRQ | DMA_NMI);
    asm("SEI");
    change_rom_bank(SAVE_BANK_NUM);
    //unlock
    *((unsigned char*) 0x8AAA) = 0xAA;
    *((unsigned char*) 0x8555) = 0x55;
    //setup
    *((unsigned char*) 0x8AAA) = 0x80;
    
    //unlock
    *((unsigned char*) 0x8AAA) = 0xAA;
    *((unsigned char*) 0x8555) = 0x55;

    //erase
    *((unsigned char*) 0x8000) = 0x30;

    i = *((unsigned char*) 0x8000);
    k = *((unsigned char*) 0x8000);
    while(i != k) {
        i = k;
        k = *((unsigned char*) 0x8000);
    }
    *dma_flags = flagsMirror;
    asm("CLI");
    pop_rom_bank();
}

void unlock_bypass() {
    *((unsigned char*) 0x8AAA) = 0xAA;
    *((unsigned char*) 0x8555) = 0x55;
    *((unsigned char*) 0x8AAA) = 0x20;
}

void lock_bypass() {
    *((unsigned char*) 0x8000) = 0x90;
    *((unsigned char*) 0x8000) = 0x00;
}

//src assumed not Flash ROM
//dest assumned to be in Flash ROM
void save_write(char* src, char* dest, char len) {
    *dma_flags = flagsMirror & ~(DMA_IRQ | DMA_NMI);
    asm("SEI");
    change_rom_bank(SAVE_BANK_NUM);
    i = 0;
    k = len;
    unlock_bypass();
    while(k) {
        *((unsigned char*) 0x8000) = 0xA0;
        dest[i] = src[i];
        while(dest[i] != src[i]) {
            
        }
        i++;
        k--;
    }
    lock_bypass();
    *dma_flags = flagsMirror;
    asm("CLI");
    pop_rom_bank();
}


#pragma optimize (pop)
#pragma code-name (pop)


