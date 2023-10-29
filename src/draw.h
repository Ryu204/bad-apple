#pragma once

#include "type.h"

inline void draw(ui8* mat) {
    // Draw by continously scan each row
    // Before scanning, the only set bit is pushed to the lower row to 
    // indicate this row will be displayed.
    // DATA_PIN, CLOCK_PIN, LATCH_PIN operations are specific to 74HC595 shift register.
    DATA_PIN = 1;
    ROW_SBIT = 0xFF;
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; ROW_SBIT = ~mat[7]; LATCH_PIN = 0;
    DATA_PIN = 0;
    for (int r = 6; r >= 0; --r)
    {
        ROW_SBIT = 0xFF;
        CLOCK_PIN = 1; CLOCK_PIN = 0;
        LATCH_PIN = 1; ROW_SBIT = ~mat[r]; LATCH_PIN = 0;
    }
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; LATCH_PIN = 0;
}