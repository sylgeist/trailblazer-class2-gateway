#include <Arduino.h>
#include "config.h"
#include "speed_out.h"
#include "speed_math.h"

#if defined(__AVR_ATmega2560__)

void speed_out_stop() {
  TCCR3A = 0;                     // disconnect OC3A
  TCCR3B = 0;                     // stop the timer
  digitalWrite(PIN_SPEED_OUT, LOW);
}

void speed_out_begin() {
  digitalWrite(PIN_SPEED_OUT, LOW);
  pinMode(PIN_SPEED_OUT, OUTPUT);
  speed_out_stop();
}

void speed_out_set_kph_x10(int16_t kph_x10) {
  uint16_t ocr = speed_ocr(speed_freq_mhz(kph_x10, SPEED_PULSES_PER_MILE));
  if (ocr == 0) { speed_out_stop(); return; }
  OCR3A = ocr;
  if (TCNT3 > ocr) TCNT3 = 0;     // avoid a long wrap when the period shrinks
  TCCR3A = _BV(COM3A0);           // toggle OC3A on compare match
  TCCR3B = _BV(WGM32) | _BV(CS32);  // CTC, prescaler 256
}

#else
void speed_out_begin() {}
void speed_out_set_kph_x10(int16_t) {}
void speed_out_stop() {}
#endif
