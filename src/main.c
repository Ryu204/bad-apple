#include "common.h"
#include "monitor.h"
#include "vga.h"

#include <string.h>

void main() {
    mnt_init();
    while (1) {
        buffer_type buffer = mnt_buffer();
        memset(buffer, 0, 64);
        vga_draw_square(buffer, 1, 2, 2, 3, 5);
        vga_draw_circle(buffer, 6, 6, 7, 1);
        mnt_swap();
        P2_1 = !P2_1; 
    }
}