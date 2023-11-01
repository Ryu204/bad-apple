#pragma once

#include "type.h"

void mnt_init();
void mnt_swap();
ui8* mnt_buffer();
void mnt_display();
void internal_refresh() __interrupt TF0_VECTOR;