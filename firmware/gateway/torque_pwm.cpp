#include <Arduino.h>
#include "config.h"
#include "torque_pwm.h"

#if defined(__AVR_ATmega2560__)

// Fast PWM, TOP = ICR1 (mode 14), prescaler 8: 16 MHz / 8 / 15625 = 128 Hz exactly.
static const uint16_t TOP = 15624;

static uint16_t to_compare(uint16_t permille) {
  if (permille > 1000) permille = 1000;
  return (uint16_t)(((uint32_t)permille * (TOP + 1UL)) / 1000UL);
}

void torque_pwm_begin() {
  digitalWrite(PIN_TORQUE_DELIVERED, LOW);
  digitalWrite(PIN_TORQUE_REQUESTED, LOW);
  pinMode(PIN_TORQUE_DELIVERED, OUTPUT);
  pinMode(PIN_TORQUE_REQUESTED, OUTPUT);
  TCCR1A = _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);
  ICR1 = TOP;
  OCR1A = 0;
  OCR1B = 0;
}

void torque_pwm_set(uint16_t delivered_permille, uint16_t requested_permille) {
  uint16_t a = to_compare(delivered_permille);
  uint16_t b = to_compare(requested_permille);
  // A compare value of 0 still gives a one-tick pulse in fast PWM, so 0 %
  // disconnects the pin from the timer and leaves it low.
  if (a == 0) { TCCR1A &= ~_BV(COM1A1); } else { OCR1A = a - 1; TCCR1A |= _BV(COM1A1); }
  if (b == 0) { TCCR1A &= ~_BV(COM1B1); } else { OCR1B = b - 1; TCCR1A |= _BV(COM1B1); }
}

void torque_pwm_stop() {
  TCCR1A &= ~(_BV(COM1A1) | _BV(COM1B1));
  digitalWrite(PIN_TORQUE_DELIVERED, LOW);
  digitalWrite(PIN_TORQUE_REQUESTED, LOW);
}

#else
void torque_pwm_begin() {}
void torque_pwm_set(uint16_t, uint16_t) {}
void torque_pwm_stop() {}
#endif
