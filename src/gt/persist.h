#define SAVE_BANK_NUM 0xFE

void clear_save_sector();
void save_write(void *src, void *dest, char len);
