#include <mcs51/8051.h>
#include "draw.h"

void delay(unsigned int ms) {
    ms *= 1000;
    while(ms--);
}

void push(ui8* a) {
    ui8 b = a[0];
    for (int i = 0; i < 8; ++i)
        a[i] = a[i + 1];
    a[7] = b;
}

void main() {
    CLOCK_PIN = 0;
    DATA_PIN = 0;
    LATCH_PIN = 0;

    ui8 img[] = {
        0b00011000,
        0b00100000,
        0b01000000,
        0b10000001,
        0b10000001,
        0b01100001,
        0b00011010,
        0b00000100
    };
    ui8 img2[] = {
        0b00010011,
        0b00110000,
        0b01100100,
        0b10000001,
        0b01001000,
        0b00011101,
        0b00111010,
        0b00000000
    };

    int t = 0;
    while (1) {
        draw(img2);
        if (t++ >= 1000) {
            t = 0;
            push(img2);
        }
    }
}