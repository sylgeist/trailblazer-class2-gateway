#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
#include "config.h"
#include "maxx_can.h"
#include "maxx_decode.h"

static MCP_CAN s_can(PIN_CAN_CS);

bool maxx_can_begin() {
  pinMode(PIN_CAN_INT, INPUT_PULLUP);
  if (s_can.begin(MCP_ANY, CAN_SPEED, CAN_CRYSTAL) != CAN_OK) return false;
  // Listen-only: the gateway never transmits on the MaxxECU bus and does not
  // acknowledge frames, so a gateway fault cannot disturb engine control.
  s_can.setMode(MCP_LISTENONLY);
  return true;
}

void maxx_can_poll(uint32_t now_ms, EngineState &s) {
  // Bounded so a flooded bus cannot starve the rest of the loop.
  for (uint8_t i = 0; i < 16 && digitalRead(PIN_CAN_INT) == LOW; i++) {
    unsigned long id = 0;
    unsigned char len = 0;
    unsigned char buf[8];
    if (s_can.readMsgBuf(&id, &len, buf) != CAN_OK) break;
    if (id & 0x80000000UL) continue;   // extended ID: not part of the MaxxECU default protocol
    maxx_decode((uint16_t)(id & 0x7FF), buf, len, now_ms, s);
  }
}
