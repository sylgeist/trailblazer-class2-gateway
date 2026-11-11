// Trailblazer Class 2 gateway
//
// Reads engine data from the MaxxECU over CAN and stands in for the factory
// PCM on the GM Class 2 (SAE J1850 VPW) bus. See README.md and docs/.
//
// As committed, the gateway only listens and reports: every output that can
// affect the vehicle is disabled in config.h.

#include <Arduino.h>
#include <avr/wdt.h>
#include "config.h"
#include "engine_state.h"
#include "maxx_can.h"
#include "maxx_decode.h"
#include "class2_frame.h"
#include "class2_bus.h"
#include "message_table.h"
#include "torque_pwm.h"
#include "speed_out.h"

static EngineState   g_engine;      // zero-initialized
static Class2BusNull g_bus;         // swap for the real backend once the interface is built
static bool          g_can_ok = false;
static uint32_t      g_last_status_ms = 0;

static void print_hex(const uint8_t *b, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    if (i) Serial.print(' ');
    if (b[i] < 0x10) Serial.print('0');
    Serial.print(b[i], HEX);
  }
}

// Outputs fall back to "off" whenever engine data is missing or stale.
static void outputs_safe() {
#if ENABLE_TORQUE_PWM
  torque_pwm_stop();
#endif
#if ENABLE_SPEED_OUT
  speed_out_stop();
#endif
}

void setup() {
  wdt_disable();
  pinMode(PIN_STATUS_LED, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Class 2 gateway starting"));

  g_can_ok = maxx_can_begin();
  Serial.println(g_can_ok ? F("CAN: MCP2515 ready (listen-only)") : F("CAN: MCP2515 NOT RESPONDING"));

  g_bus.begin();

#if ENABLE_TORQUE_PWM
  torque_pwm_begin();
#endif
#if ENABLE_SPEED_OUT
  speed_out_begin();
#endif

  wdt_enable(WDTO_1S);
}

void loop() {
  wdt_reset();
  const uint32_t now = millis();

  if (g_can_ok) maxx_can_poll(now, g_engine);
  const bool fresh = maxx_data_fresh(g_engine, now, CAN_TIMEOUT_MS);

  // ---- Class 2 receive: report everything heard on the bus ----
  uint8_t raw[CLASS2_MAX_FRAME];
  uint8_t n = g_bus.receive(raw, sizeof(raw));
  if (n) {
    Class2Frame f;
    Class2DecodeResult r = class2_decode(raw, n, f);
    Serial.print(F("C2 RX: "));
    print_hex(raw, n);
    if (r != CLASS2_OK) { Serial.print(F("  !err ")); Serial.print((int)r); }
    Serial.println();
  }

  // ---- Class 2 transmit: periodic messages, only with live engine data ----
#if ENABLE_CLASS2_TX
  if (fresh) {
    int8_t idx = scheduler_poll(g_messages, g_message_count, now);
    if (idx >= 0) {
      Class2Frame f;
      uint8_t out[CLASS2_MAX_FRAME];
      message_build(g_messages[idx], GATEWAY_NODE_ADDR, g_engine, f);
      uint8_t len = class2_encode(f, out);
      if (len) g_bus.send(out, len);
    }
  }
#endif

  // ---- Hardwired EBCM signals ----
  if (fresh) {
#if ENABLE_SPEED_OUT
    speed_out_set_kph_x10(g_engine.speed_kph_x10);
#endif
#if ENABLE_TORQUE_PWM
    // Duty-cycle mapping is not characterized yet; hold both outputs low.
    torque_pwm_set(0, 0);
#endif
  } else {
    outputs_safe();
  }

  // ---- 1 Hz status line ----
  if ((uint32_t)(now - g_last_status_ms) >= 1000) {
    g_last_status_ms = now;
    digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
    Serial.print(F("status can="));
    Serial.print(fresh ? F("fresh") : F("STALE"));
    Serial.print(F(" rpm="));    Serial.print(g_engine.rpm);
    Serial.print(F(" clt_x10=")); Serial.print(g_engine.coolant_c_x10);
    Serial.print(F(" kph_x10=")); Serial.println(g_engine.speed_kph_x10);
  }
}
