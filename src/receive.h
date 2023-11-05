#pragma once

#include "monitor.h"

void rcv_init();
void rcv_set_buffer(buffer_type buf);
ui8 rcv_done();
void rcv_request();
void internal_write_byte() __interrupt (SI0_VECTOR);