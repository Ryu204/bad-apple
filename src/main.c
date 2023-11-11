/////////////////////////////////////////////////////
//
// bad-apple on STC89C52 microcontroller
// Copyright (C) 2023 Nguyen Anh Bao (nguyenanhbao2356@gmail.com)
// The source code is delivered under MIT license.
//
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Headers
/////////////////////////////////////////////////////
#include <stdint.h>
#include <string.h>
#include <mcs51/8051.h>

// P1 becomes high byte indicator for XRAM location.
// Default indicator is P2 (which controls the LEDs), and we do not want that.
__sfr __at (0x90) _XPAGE;

/////////////////////////////////////////////////////
// Monitor section
// The display system relies on 74HC595 shift register to scan each line.
/////////////////////////////////////////////////////
#define DATA_PIN    P3_4
#define CLOCK_PIN   P3_6
#define LATCH_PIN   P3_5
#define ROW_SBIT    P0
#define COLOR_CNT   0x04    // Number of brightness levels
#define DPL_INT     0xFF    // Time interval of timer0 interrupt
#define DPL_MTP     0x1D    // Number of interrupts before actually displaying the data
#define LINE_DELAY  0x2F    // Lit duration of each row

// The buffer holds 2 64-byte arrays at once to support double buffering.
uint8_t __pdata display_buffer[128] = {0};
// Represent the current active buffer (0 or 1)
uint8_t display_index = 0;
// A value cycling from 1 to `COLOR_CNT`, used for PWM
uint8_t pass = 0;
// Used to multiply interrupt time (8-bit timer's is very short)
uint8_t counter = 0;
uint8_t rendering = 0;
uint8_t allow_display = 1;

/////////////////////////////////////////////////////
/// \brief Initialize neccessary variables for display
///
/////////////////////////////////////////////////////
inline void init_display() {
    ET0 = 1;                // Enable timer0
    TH0 = 0xFF - DPL_INT;   // Set interrupt interval
    TMOD |= 0x02;           // timer0 8-bit auto reload
    TR0 = 1;                // Start timer0

    CLOCK_PIN = 0;
    LATCH_PIN = 0;
    DATA_PIN = 0;
    ROW_SBIT = 0xFF;
}

/////////////////////////////////////////////////////
/// \brief Display the data in active buffer to LED matrix
/// \note Don't call this function directly, it's meant for periodical updates by timer0
///
/////////////////////////////////////////////////////
inline void display() {
    rendering = 1;
    pass = pass == COLOR_CNT ? 1 : pass + 1;
    uint8_t* start = display_index == 0 ? display_buffer + 56 : display_buffer + 120;
    uint8_t* end = start - 64;
    // Enable the first column
    DATA_PIN = 1;
    CLOCK_PIN = 1; CLOCK_PIN = 0;
    LATCH_PIN = 1; LATCH_PIN = 0;
    DATA_PIN = 0;
    uint8_t row_data = 0xFF;
    for (uint8_t* line = start; line != end; line -= 8) {
        row_data = 0xFF;
        for (uint8_t index = 0; index < 8; ++index) {
            if (line[7 - index] >= pass) {
                row_data &= ~(0x01 << index);
            }
        }
        ROW_SBIT = row_data;
        for (uint8_t i = 0; i < LINE_DELAY; ++i);
        ROW_SBIT = 0xFF;
        CLOCK_PIN = 1; CLOCK_PIN = 0;
        LATCH_PIN = 1; LATCH_PIN = 0;
    }
    rendering = 0;
}

/////////////////////////////////////////////////////
/// \brief Immediately render the scene, bypass wait time
/// \note Unlike `display()`, this function can be called directly
///
/////////////////////////////////////////////////////
inline void active_display() {
    TR0 = 0;        // Stop interrupt display
    TF0 = 0;
    display();
    TR0 = 1;        // Reset interrupt display
    TL0 = TH0;
    counter = 1;
}

/////////////////////////////////////////////////////
/// \brief After a fixed amount of intterupts, call the `display()` function
///
/////////////////////////////////////////////////////
void display_interupt() __interrupt (TF0_VECTOR) {
    if (++counter == DPL_MTP) {
        if (allow_display)
            display();
        counter = 0;
    }
}

/////////////////////////////////////////////////////
/// \brief Get the available data buffer
/// \return Pointer to the beginning of the inactive buffer
///
/////////////////////////////////////////////////////
inline uint8_t* get_display_buffer() {
    return display_index == 0 ? display_buffer + 64 : display_buffer;
}

/////////////////////////////////////////////////////
/// \brief Swap active and inactive buffers
///
/////////////////////////////////////////////////////
inline void swap() {
    display_index = 1 - display_index;
}

/////////////////////////////////////////////////////
/// \brief Enable or disable the display
///
/////////////////////////////////////////////////////
inline void display_setting(uint8_t on) {
    allow_display = on;
}

/////////////////////////////////////////////////////
// Serial section
/////////////////////////////////////////////////////
#define LINE_SIGNAL     0xFE        // Signal to request a new display line
#define FRAME_SIGNAL    0xFF        // Signal to request a new display frame
uint8_t* line_location;             // The line buffer that will be written to
volatile uint8_t line_index = 0;    // The current index in line buffer

/////////////////////////////////////////////////////
/// \brief Initialize neccessary variables for serial communication
///
/////////////////////////////////////////////////////
inline void init_uart() {
    ES = 1;         // Enable serial interrupt
    TMOD |= 0x20;   // Set timer1 to 8-bit auto reload
    PCON |= SMOD;   // Double baudrate
    SCON |= 0x50;   // Config serial port mode
    TH1 = 250;      // Baudrate 19200, by double-speed (6T) and double baudrate
    TR1 = 1;        // Start timer1
}

/////////////////////////////////////////////////////
/// \brief Send a byte to computer
///
/////////////////////////////////////////////////////
inline void send(uint8_t signal) {
    TI = 0;
    SBUF = signal;
    while (TI == 0);
    TI = 0;
}

/////////////////////////////////////////////////////
/// \brief Send a ready-to-read-line signal to computer
///
/////////////////////////////////////////////////////
inline void request_line(uint8_t* buffer) {
    line_location = buffer;
    line_index = 0;
    send(LINE_SIGNAL);
}

/////////////////////////////////////////////////////
/// \brief Interrupt writes the received variable into line buffer
///
/////////////////////////////////////////////////////
void line_receive_listener() __interrupt (SI0_VECTOR) {
    if (RI == 0)    // If the interrupt was by sending, not receiving
        return;
    line_location[line_index++] = SBUF;
    RI = 0;
}

/////////////////////////////////////////////////////
/// \brief Indicate the current state of the buffer
/// \return True if all 8 bytes were written to the buffer, false otherwise
///
/////////////////////////////////////////////////////
inline uint8_t got_line() {
    return line_index >= 8;
}

/////////////////////////////////////////////////////
// Driver functions
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
/// \brief Initialize neccessary variables
///
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
        // At each loop, the following steps are executed:
        //  * retrieve buffer for writing and clear its content
        //  * request to read a frame, then read each line one by one into the buffer
        //  * ensure the display is not lagged behind
        //  * swap the fetched buffer to be the active one
        uint8_t* buffer = get_display_buffer();
        memset(buffer, 0, 64);
        send(FRAME_SIGNAL);
        for (uint8_t row = 0; row < 8; ++row) {
            while (rendering != 0);
            display_setting(0);
            request_line(buffer + row * 8);
            while (!got_line());
            display_setting(1);
            active_display();
        }
        swap();
    }
}