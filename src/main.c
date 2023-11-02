#include "common.h"
#include "monitor.h"
#include "vga.h"

#include <string.h>

void main() {
    mnt_init();
    long t = 0;
    while (1) {
        if (++t == 15000) {
            buffer_type buffer = mnt_buffer();
            memset(buffer, 0, 64);
            vga_draw_square(buffer, 1, 2, 2, 3, 1);
            vga_draw_circle(buffer, 6, 6, 5, 1);
            mnt_swap();
            t = 0;
        }
    }
}