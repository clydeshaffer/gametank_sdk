#include "gametank.h"

unsigned char romBankMirror;

void SwitchRomBank(unsigned char x);

void ChangeRomBank(unsigned char banknum) {
    if(banknum == romBankMirror)
        return;
    romBankMirror = banknum;
    via[ORA] = 0;
    via[ORA] = !!(banknum & 64) << 1;
    via[ORA] |= 1;
    via[ORA] = !!(banknum & 32) << 1;
    via[ORA] |= 1;
    via[ORA] = !!(banknum & 16) << 1;
    via[ORA] |= 1;
    via[ORA] = !!(banknum & 8) << 1;
    via[ORA] |= 1;
    via[ORA] = !!(banknum & 4) << 1;
    via[ORA] |= 1;
    via[ORA] = banknum & 2;
    via[ORA] |= 1;
    via[ORA] = !!(banknum & 1) << 1;
    via[ORA] |= 1;
    via[ORA] |= 4;
    via[ORA] = 0;
}