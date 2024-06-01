#include "gametank.h"

//must be power of 2
#define BANK_STACK_SIZE 8
//must be BANK_STACK_SIZE-1
#define BANK_WRAP_MASK 7

unsigned char romBankMirror;
unsigned char romBankStack[BANK_STACK_SIZE];
unsigned char romBankStackIdx;

void bank_shift_out(unsigned char banknum);

void change_rom_bank(unsigned char banknum) {
    romBankStackIdx = (romBankStackIdx + 1) & BANK_WRAP_MASK;
    romBankStack[romBankStackIdx] = romBankMirror;
    if(banknum == romBankMirror)
        return;
    bank_shift_out(banknum);
}

void pop_rom_bank() {
    bank_shift_out(romBankStack[romBankStackIdx]);
    if(romBankStackIdx == 0)
        romBankStackIdx = BANK_STACK_SIZE;
    --romBankStackIdx;
}