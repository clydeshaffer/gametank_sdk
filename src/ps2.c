#include "gt/gametank.h"
#include "ps2.h"
#include "util.h"

#define CLEAR_INHIBIT (1<<5)
#define WRITE_STROBE (1<<4)
#define READ_STROBE (1<<3)
#define PACKET_DONE (1<<6)

#define SET(bit) via[ORA] |= bit
#define CLEAR(bit) via[ORA] &= ~bit

char ledMask = 0;
char oldLedMask = 0;
static char tmp;

void exbus_select_device(char addr) {
    via[0x0C] = 0b11101110;
    tmp = via[DDRB];
    via[DDRB] = 0xFF;
    via[ORB] = addr;
    via[0x0C] = 0b11001100;
    via[DDRB] = tmp;
}

void ps2_init() {
    exbus_select_device(0);

    via[DDRA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
    via[DDRA] &= ~PACKET_DONE;
    via[ORA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
    via[DDRB] = 0;
}

//0, 1, 2, 3, 4, 5, 6, 7
//0, 7, 6, 5, 4, 1, 2, 3
//only needed on the breadboard
//which was wired for physical not digital convenience
//remove for PCB version with consistent ordering
char scramble(char b) {
    tmp = 0;
    tmp |= (b & 0b10001000);
    //tmp |= (b & 1) << 6;
    //tmp |= (b & 2) << 4;
    //tmp |= (b & 4) << 2;
    //tmp |= (b & (16|32|64)) >> 4;
    tmp |= (b & (1|2|4)) << 4;
    tmp |= (b & 16) >> 2;
    tmp |= (b & 32) >> 4;
    tmp |= (b & 64) >> 6;
    return tmp;
}

char read_byte() {
    CLEAR(READ_STROBE);
    SET(READ_STROBE);
    CLEAR(READ_STROBE);
    tmp = via[ORB];
    SET(READ_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    return tmp;
}

void send_byte(char b) {
    via[ORA] |= PACKET_DONE;
    via[DDRA] |= PACKET_DONE;
    via[DDRB] = 0xFF;
    via[ORB] = b;//scramble(b);
    delayMicroseconds(100);
    CLEAR(WRITE_STROBE);
    via[DDRA] &= ~PACKET_DONE;
    via[ORA] &= ~PACKET_DONE;;
    SET(WRITE_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    via[DDRB] = 0;
}

char wait_for_packet() {
  int cnt = 0;
  while(!(via[ORA] & PACKET_DONE)) {
    delayMicroseconds(100);
    ++cnt;
    if(cnt > 500) {
      return 0;
    }
  }
  return 1;
}

void clear_clock_inhibit() {
  CLEAR(CLEAR_INHIBIT);
  SET(CLEAR_INHIBIT);
}

int send_byte_and_get_response(unsigned char b, unsigned char* buf, int lim) {
  int response_bytes = 0;

  send_byte(b);
  wait_for_packet();
  tmp = read_byte();
  clear_clock_inhibit();
  while((response_bytes < lim) && wait_for_packet()) {
    tmp = read_byte();
    clear_clock_inhibit();
    buf[response_bytes] = tmp;
    response_bytes++;
  }
  return response_bytes;
}

void handle_led_keys(unsigned char lastRead) {
    if((lastRead & 0xFF) == 0x77) {
        ledMask ^= 2;
    }

    if((lastRead & 0xFF) == 0x58) {
        ledMask ^= 4;
    }

    if((lastRead & 0xFF) == 0x7E) {
        ledMask ^= 1;
    }

    if(ledMask != oldLedMask) {
        oldLedMask = ledMask;
        send_byte(0xED);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
        clear_clock_inhibit();
        wait_for_packet();
        clear_clock_inhibit();
        send_byte(ledMask);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
    }
}

char ps2_data_ready() {
    return via[ORA] & PACKET_DONE;
}