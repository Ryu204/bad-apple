#include "common.h"
#include "monitor.h"
#include "vga.h"
#include "receive.h"
#include <string.h>

void main() {
    mnt_init();
    rcv_init();
    while (1) {
        buffer_type buffer = mnt_buffer();
        memset(buffer, 0, 64);
        // vga_draw_square(buffer, 1, 0, 2, 5, 1);
        // vga_draw_circle(buffer, 6, 6, 5, 8);
        //  vga_draw_lol(buffer);
        rcv_on(buffer);
        while (!rcv_done());
        mnt_swap();
    }
}

// void ISR() __interrupt SI0_VECTOR {
//     //P2 = SBUF;
//     if (RI == 1) {
//         P2 = SBUF;
//         RI = 0;
//     }
// }

// void main() {
//     rcv_init();
//     rcv_on(NULL);
//     while (1) {
//         // while (!RI);
//         // RI = 0;
//         // P2 = SBUF;
//     }
// }