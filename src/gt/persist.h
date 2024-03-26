#define SAVE_BANK_NUM 0xFE

void clear_save_sector();
void save_write(char* src, char* dest, char len);