#ifndef BANKING_H
#define BANKING_H

extern unsigned char romBankMirror;
void change_rom_bank(unsigned char banknum);

#define BANK_COMMON 0
#define BANK_TILES 1
#define BANK_MONSTERS 2
#define BANK_INIT 3
#define BANK_MONSTERS2 4
#define BANK_MONSTERS3 5

#endif