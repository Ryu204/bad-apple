#include "type.h"
#include <mcs51/8051.h>
#include <stdint.h>
#include <string.h>

#define ui8 uint8_t
__sfr __at (0x90) _XPAGE; // P1 becomes high byte indicator for XRAM location

/////////////////////////////////////////////////////
// Monitor section
/////////////////////////////////////////////////////
#define DATA_PIN    P3_4
#define CLOCK_PIN   P3_6
#define LATCH_PIN   P3_5
#define ROW_SBIT    P0
#define COLOR_CNT   0x04
#define DPL_INT     0xFF
#define DPL_MTP     0x1D
#define LINE_DELAY  0x2F

ui8 __pdata display_buffer[128] = {
    0,0,0,0,0,0,0,0,
    0,0,2,2,2,2,0,0,
    0,2,1,1,1,1,2,2,
    2,1,1,8,1,8,1,2,
    2,8,1,1,1,1,1,2,
    2,1,8,8,8,8,8,2,
    0,2,1,8,8,8,1,2,
    0,0,2,2,2,2,2,0
};
ui8 display_index = 0;
ui8 pass = 0;

inline void init_display() {
    ET0 = 1;                // Enable timer0
    TH0 = 0xFF - DPL_INT;   // Set interupt interval
    TMOD |= 0x02;           // timer0 8-bit auto reload
    TR0 = 1;                // Start timer0s

    CLOCK_PIN = 0;
    LATCH_PIN = 0;
    DATA_PIN = 0;
    ROW_SBIT = 0xFF;
}

inline void display() {
    pass = pass == COLOR_CNT ? 1 : pass + 1;
    ui8* start = display_index == 0 ? display_buffer + 56 : display_buffer + 120;
    ui8* end = start - 64;
    // Enable to the first column
    DATA_PIN = 1;
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; LATCH_PIN = 0;
    DATA_PIN = 0;
    ui8 row_data = 0xFF;
    for (ui8* line = start; line != end; line -= 8) {
        row_data = 0xFF;
        for (ui8 index = 0; index < 8; ++index) {
            if (line[7 - index] >= pass) {
                row_data &= ~(0x01 << index);
            }
        }
        ROW_SBIT = row_data;
        for (ui8 i = 0; i < LINE_DELAY; ++i);
        ROW_SBIT = 0xFF;
        CLOCK_PIN = 1; CLOCK_PIN = 0;
        LATCH_PIN = 1; LATCH_PIN = 0;
    }
}

ui8 counter;
void display_interupt() __interrupt (TF0_VECTOR) {
    if (++counter == DPL_MTP) {
        display();
        counter = 0;
    }
}

inline ui8* get_display_buffer() {
    return display_index == 0 ? display_buffer + 64 : display_buffer;
}

inline void swap() {
    display_index = 1 - display_index;
}

/////////////////////////////////////////////////////
// Serial section
/////////////////////////////////////////////////////
#define LINE_SIGNAL     0xFE
#define FRAME_SIGNAL    0xFF
ui8* line_location;
volatile ui8 line_index = 0;

inline void init_uart() {
    ES = 1;         // Enable serial interrupt
    TMOD |= 0x20;   // Set timer1 to 8-bit auto reload
    PCON |= SMOD;   // Double baudrate
    SCON |= 0x50;   // Config serial port mode
    TH1 = 250;      // 19200 Hz by double-speed (6T) and double baudrate
    TR1 = 1;        // Start timer1
}

inline void send(ui8 signal) {
    TI = 0;
    SBUF = signal;
    while (TI == 0);
    TI = 0;
}

inline void request_line(ui8* buffer) {
    line_location = buffer;
    line_index = 0;
    send(LINE_SIGNAL);
}

void line_receive_listener() __interrupt (SI0_VECTOR) {
    if (RI == 0) // If the interrupt was sending, not receiving
        return;
    line_location[line_index++] = SBUF;
    RI = 0;
}

inline ui8 got_line() {
    return line_index >= 8;
}
/////////////////////////////////////////////////////
// Driver function
/////////////////////////////////////////////////////
inline void init() {
    EA = 1;
    init_display();
    init_uart();
    PT0 = 0;        // Display has low priority
    PS = 1;         // Serial has high priority
}

void main() {
    init();
    while (1) {
        ui8* buffer = get_display_buffer();
        memset(buffer, 0, 64);
        send(FRAME_SIGNAL);
        for (ui8 row = 0; row < 8; ++row) {
            request_line(buffer + row * 8);
            while (!got_line());
        }
        swap();
    }
}