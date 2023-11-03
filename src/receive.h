#pragma once

#include "monitor.h"

void rcv_init();
void rcv_on(buffer_type buf);
void rcv_off();
ui8 rcv_done();
void internal_write_byte() __interrupt SI0_VECTOR;