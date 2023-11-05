#include "common.h"
#include "monitor.h"
#include "vga.h"
#include "receive.h"
#include <string.h>

void init() {
    mnt_init();
    rcv_init();
    PT0 = 0;
    PS = 1;
}

void main() {
    init();
    while (1) {
        buffer_type buffer = mnt_buffer();
        memset(buffer, 0, 64);
        rcv_set_buffer(buffer);
        rcv_request();
        while (!rcv_done());
        mnt_swap();
    }
}