#pragma once

#include "type.h"

#define     buffer_type ui8*

void mnt_init();
void mnt_swap();
buffer_type mnt_buffer();
void mnt_display();
void internal_refresh() __interrupt TF0_VECTOR;