#ifndef PERSIST_H
#define PERSIST_H

void clear_save_sector();
void save_write(void *src, void *dest, char len);

#endif