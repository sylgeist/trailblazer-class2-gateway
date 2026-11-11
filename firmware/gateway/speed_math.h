// Pure math for the rear-speed square wave (Timer3, prescaler 256 = 62,500 ticks/s).
#pragma once
#include <stdint.h>

// Output frequency in millihertz for a road speed (km/h x10).
inline uint32_t speed_freq_mhz(int16_t kph_x10, uint32_t pulses_per_mile) {
  if (kph_x10 <= 0) return 0;
  // Hz = (kph / 1.609344) * ppm / 3600  ->  mHz = kph_x10 * ppm * 1e6 / 57,936,384
  return (uint32_t)(((uint64_t)(uint16_t)kph_x10 * pulses_per_mile * 1000000ULL) / 57936384ULL);
}

// Timer compare value for a toggling output at `freq_mhz`.
// Returns 0 when the speed is too low to represent (below 0.5 Hz): output off.
inline uint16_t speed_ocr(uint32_t freq_mhz) {
  if (freq_mhz < 500) return 0;
  uint32_t ticks = 62500000UL / (2UL * freq_mhz);   // half-period in timer ticks
  if (ticks < 2) ticks = 2;
  if (ticks > 65536UL) ticks = 65536UL;
  return (uint16_t)(ticks - 1);
}
