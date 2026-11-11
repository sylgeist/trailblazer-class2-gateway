// Latest engine data received from the MaxxECU over CAN.
// Values keep the MaxxECU's fixed-point scaling so no floats are needed.
#pragma once
#include <stdint.h>

struct EngineState {
  int16_t  rpm;              // rpm
  int16_t  throttle_x10;     // % x10
  int16_t  map_kpa_x10;      // kPa x10
  int16_t  speed_kph_x10;    // km/h x10
  int16_t  battery_v_x100;   // V x100
  int16_t  iat_c_x10;        // deg C x10
  int16_t  coolant_c_x10;    // deg C x10
  int16_t  gear;
  int16_t  oil_kpa_x10;      // kPa x10
  uint32_t last_fast_ms;     // time of the last 50 Hz frame (0x520)
  uint32_t last_slow_ms;     // time of the last 10 Hz frame (0x530)
};
