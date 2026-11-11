// Rear-speed square wave to the EBCM on Timer3 (OC3A). The Mega pin is 5 V
// logic; the EBCM input expects a high-side drive, so an external transistor
// stage is required between this pin and the vehicle (docs/pins-and-timers.md).
#pragma once
#include <stdint.h>

void speed_out_begin();                    // output low, timer stopped
void speed_out_set_kph_x10(int16_t kph_x10);
void speed_out_stop();
