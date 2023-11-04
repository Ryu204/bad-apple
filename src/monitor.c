#include "monitor.h"
#include <string.h>

#define DATA_PIN    P3_4
#define CLOCK_PIN   P3_6
#define LATCH_PIN   P3_5
#define ROW_SBIT    P0

#define COLOR_CNT   0x04
#define MULTIPLIER  0x1D
#define TL_VAL      0x00

ui8 __pdata buffer[128];
ui8 index_current = 0;
ui8 pass = 0;

void process_row(ui8 start, ui8* data) {
    *data = 0xFF;
    static ui8 mask[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
    for (ui8 i = 8; i > 0; --i)
        if (buffer[start + i - 1] >= pass)
            *data &= mask[i - 1];
}

void mnt_init() {
    // Config Timer0
    EA = 1;
    ET0 = 1;
    TH0 = TL_VAL;
    TMOD |= 0x02; // 8 bit auto reload
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
    pass = pass == COLOR_CNT ? 1 : pass + 1;
    ui8 index = index_current == 0 ? 56 : 120;
    
    ui8 row_data;
    process_row(index, &row_data);
    DATA_PIN = 1;
    ROW_SBIT = 0xFF;
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; ROW_SBIT = row_data; LATCH_PIN = 0;
    DATA_PIN = 0;

    for (ui8 r = 0; r < 7; ++r) {
        index -= 8;
        process_row(index, &row_data);
        ROW_SBIT = 0xFF;
        CLOCK_PIN = 1; CLOCK_PIN = 0;
        LATCH_PIN = 1; ROW_SBIT = row_data; LATCH_PIN = 0;
    }
    for (ui8 i = 0; i < 50; ++i);
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; LATCH_PIN = 0;
}

ui8 counter = 0;
void internal_refresh() __interrupt (TF0_VECTOR) {
    if (++counter == MULTIPLIER) {
        mnt_display();
        counter = 0;
    }
}