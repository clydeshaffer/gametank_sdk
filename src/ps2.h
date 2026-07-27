//Personal System 2 not Play Station 2
#include "gametank.h"

extern char ledMask;

void ps2_init();

char read_byte();

void send_byte(char b);

char wait_for_packet();

int send_byte_and_get_response(unsigned char b, unsigned char* buf, int lim);

void handle_led_keys(unsigned char lastRead);

char ps2_data_ready();