#pragma once

#include <mcs51/8051.h>
#include "type.h"

 __sfr __at (0x90) _XPAGE; // P1 becomes high byte indicator for XRAM location

inline void add_clamp(ui8* x, ui8 a) {
    if ((*x) + 0 + a >= 255)
        *x = 255;
    else
        *x += a;
}