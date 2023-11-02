#pragma once

#include "type.h"
#include "monitor.h"

void vga_draw_square(buffer_type buffer, ui8 t, ui8 l, ui8 w, ui8 h, ui8 color);
void vga_draw_circle(buffer_type buffer, ui8 x, ui8 y, ui8 r, ui8 color);
void vga_draw_lol(buffer_type buffer);