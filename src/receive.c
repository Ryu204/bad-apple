#include "receive.h"
#include <mcs51/8051.h>

buffer_type rcv_buffer;
ui8 buffer_index;
ui8 done;

void rcv_init() {
    EA = 1;
    ES = 1;
    TMOD |= 0x20;
    PCON |= SMOD;
    SCON |= 0x40;
    TH1 = 250;
    TR1 = 1;
}

void rcv_on(buffer_type buf) {
    SCON |= 0x10;
    rcv_buffer = buf;
    buffer_index = 0;
    done = 0;
}

void rcv_off() {
    SCON &= ~0x10;
}

ui8 rcv_done() {
    return done;
}

void internal_write_byte() __interrupt SI0_VECTOR {
    rcv_buffer[buffer_index++] = SBUF;
    P2 = SBUF;
    if (buffer_index == 64)
        done = 1;
    RI = 0;
}