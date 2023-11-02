#include "vga.h"
#include "common.h"

void vga_draw_square(buffer_type buffer, ui8 t, ui8 l, ui8 w, ui8 h, ui8 color) {
    for (ui8 r = t; r - t <= h; ++r)
        for (ui8 c = l; c - l <= w; ++c)
            add_clamp(buffer + r * 8 + c, color);
}

void vga_draw_circle(buffer_type buffer, ui8 x, ui8 y, ui8 r, ui8 color) {
    for (int i = 0; i < 64; ++i) {
        int dx = i % 8 - x, dy = i / 8 - y;
        if (dx * dx + dy * dy <= r * r)
            add_clamp(buffer + i, color);
    }
}