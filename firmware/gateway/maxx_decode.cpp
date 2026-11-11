#include "maxx_decode.h"

static int16_t le16(const uint8_t *d, uint8_t off) {
  return (int16_t)((uint16_t)d[off] | ((uint16_t)d[off + 1] << 8));
}

bool maxx_decode(uint16_t id, const uint8_t *d, uint8_t len, uint32_t now_ms, EngineState &s) {
  if (len < 8) return false;
  switch (id) {
    case 0x520:
      s.rpm          = le16(d, 0);
      s.throttle_x10 = le16(d, 2);
      s.map_kpa_x10  = le16(d, 4);
      s.last_fast_ms = now_ms;
      return true;
    case 0x522:
      s.speed_kph_x10 = le16(d, 6);
      return true;
    case 0x530:
      s.battery_v_x100 = le16(d, 0);
      s.iat_c_x10      = le16(d, 4);
      s.coolant_c_x10  = le16(d, 6);
      s.last_slow_ms   = now_ms;
      return true;
    case 0x536:
      s.gear        = le16(d, 0);
      s.oil_kpa_x10 = le16(d, 4);
      return true;
    default:
      return false;
  }
}

bool maxx_data_fresh(const EngineState &s, uint32_t now_ms, uint32_t timeout_ms) {
  if (s.last_fast_ms == 0 || s.last_slow_ms == 0) return false;
  return (uint32_t)(now_ms - s.last_fast_ms) <= timeout_ms &&
         (uint32_t)(now_ms - s.last_slow_ms) <= timeout_ms * 4;
}
