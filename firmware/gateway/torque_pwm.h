// 128 Hz PWM pair to the EBCM (delivered torque, requested torque) on Timer1.
// What each duty cycle MEANS is not yet characterized; see docs/message-table.md.
#pragma once
#include <stdint.h>

void torque_pwm_begin();                                            // starts with both outputs low
void torque_pwm_set(uint16_t delivered_permille, uint16_t requested_permille);  // 0-1000
void torque_pwm_stop();                                             // both outputs low
