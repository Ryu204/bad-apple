#include "receive.h"
#include <mcs51/8051.h>

buffer_type rcv_buffer;
ui8 buffer_index;

void rcv_init() {
    EA = 1;
    ES = 1;
    TMOD |= 0x20;
    PCON |= SMOD;
    SCON |= 0x50;
    TH1 = 250;
    TR1 = 1;
}

void rcv_set_buffer(buffer_type buf) {
    rcv_buffer = buf;
    buffer_index = 0;
}

ui8 rcv_done() {
    return buffer_index > 63;
}

void rcv_request() {
    TI = 0;
    SBUF = 0xFF;
    while (TI == 0);
    TI = 0;
}

void internal_write_byte() __interrupt (SI0_VECTOR) {
    if (RI == 0 || buffer_index > 63)
        return;
    rcv_buffer[buffer_index++] = SBUF;
    RI = 0;
}