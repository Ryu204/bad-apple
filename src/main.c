#include "common.h"
#include "monitor.h"
#include "vga.h"

#include <string.h>

void main() {
    mnt_init();
    while (1) {
        buffer_type buffer = mnt_buffer();
        memset(buffer, 0, 64);
        // vga_draw_square(buffer, 1, 0, 2, 5, 1);
        // vga_draw_circle(buffer, 6, 6, 5, 8);
        vga_draw_lol(buffer);
        mnt_swap();
        P2_1 = !P2_1; 
    }
}