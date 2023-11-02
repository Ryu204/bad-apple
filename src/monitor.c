#include "monitor.h"
#include <string.h>

#define DATA_PIN    P3_4
#define CLOCK_PIN   P3_6
#define LATCH_PIN   P3_5
#define ROW_SBIT    P0

#define COLOR_CNT   8
#define MULTIPLIER  10
#define TL_VAL      0x00

ui8 __pdata buffer[128];
ui8 index_current = 0;
int pass = 0;

inline ui8 process_row(ui8 start) {
    ui8 res = 0xFF;
    for (ui8 i = 0; i < 8; ++i)
        if (buffer[start + 7 - i] >= pass)
            res &= ~(1 << i);
    return res;
}

void mnt_init() {
    // Config Timer0
    EA = 1;
    ET0 = 1;
    TH0 = (ui8)TL_VAL; TL0 = 0;
    TMOD = 0x02;
    TR0 = 1;

    CLOCK_PIN = 0;
    LATCH_PIN = 0;
    DATA_PIN = 0;
}

void mnt_swap() {
    index_current = 1 - index_current;
}

buffer_type mnt_buffer() {
    return buffer + (1 - index_current) * 64;
}

inline void mnt_display() {
    P2_0 = !P2_0;
    pass = pass == COLOR_CNT ? 1 : pass + 1;
    ui8 index = index_current == 0 ? 56 : 120;
    
    ui8 row_data = process_row(index);
    DATA_PIN = 1;
    ROW_SBIT = 0xFF;
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; ROW_SBIT = row_data; LATCH_PIN = 0;
    DATA_PIN = 0;

    for (ui8 r = 0; r < 7; ++r) {
        index -= 8;
        row_data = process_row(index);
        ROW_SBIT = 0xFF;
        CLOCK_PIN = 1; CLOCK_PIN = 0;
        LATCH_PIN = 1; ROW_SBIT = row_data; LATCH_PIN = 0;
    }
    for (ui8 i = 0; i < 50; ++i);
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; LATCH_PIN = 0;
}

unsigned int counter = 0;
void internal_refresh() __interrupt TF0_VECTOR {
    if (++counter == MULTIPLIER) {
        mnt_display();
        counter = 0;
    }
}