#include "common.h"
#include "monitor.h"

void main() {
    mnt_init();
    long t = 0;
    while (1) {
        ui8* buffer = mnt_buffer();
        // do shit with buffer
        // mnt_swap();
        if (++t == 15000) {
            mnt_swap();
            t = 0;
        }
    }
}