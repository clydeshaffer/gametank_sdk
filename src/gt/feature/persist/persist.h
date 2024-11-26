#ifndef PERSIST_H
#define PERSIST_H

#include "../../../gen/modules_enabled.h"

#ifndef ENABLE_MODULE_PERSIST
#error "Module PERSIST included but not enabled!"
#endif

void clear_save_sector();
void save_write(void *src, void *dest, char len);

#endif