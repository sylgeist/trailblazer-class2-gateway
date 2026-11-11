// Decoder for the MaxxECU default CAN protocol (500 kbit/s, 11-bit IDs,
// little-endian int16 fields). Field map: docs/protocol.md.
#pragma once
#include <stdint.h>
#include "engine_state.h"

// Updates `s` from one CAN frame. Returns true if the ID was one we use.
bool maxx_decode(uint16_t id, const uint8_t *d, uint8_t len, uint32_t now_ms, EngineState &s);

// True when both the fast and slow frames have arrived within `timeout_ms`.
bool maxx_data_fresh(const EngineState &s, uint32_t now_ms, uint32_t timeout_ms);
