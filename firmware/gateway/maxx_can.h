// MCP2515 (Adafruit CAN FeatherWing #5709) receive path for MaxxECU data.
#pragma once
#include "engine_state.h"

bool maxx_can_begin();                                   // false if the MCP2515 does not respond
void maxx_can_poll(uint32_t now_ms, EngineState &s);     // drain pending frames
