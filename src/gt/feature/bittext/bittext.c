#include "../../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_BITTEXT

#include "../../banking.h"
#include "../../../gen/assets/sdk_default.h"

#include "bittext.h"
#include "font.h"
#include "gametank.h"

char bittext_color = 32;
char bittext_bg = 7;

void bittext_write_str(char* c) {
    push_rom_bank();
    change_rom_bank(BANK_sdk_default);
    while(*c != 0) {
        PROFILER_START(0);
        bittext_draw_char(*c);
        PROFILER_END(0);
        ++c;
    }
    pop_rom_bank();
}
#endif