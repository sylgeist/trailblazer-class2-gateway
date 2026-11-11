#include "message_table.h"

// Add entries here once they are confirmed from a stock-truck capture, e.g.
//
//   static uint8_t fill_example(const EngineState &s, uint8_t *data) { ... }
//   { class2_make_header(4, false, 1), 0x00 /*functional ID*/, 100, fill_example, 0 },
//
// A zero-length array is not legal C++, so the table holds one inert
// placeholder with a null fill function; g_message_count stays 0.
PeriodicMessage g_messages[] = {
  { 0, 0, 0, 0, 0 },
};
const uint8_t g_message_count = 0;

int8_t scheduler_poll(PeriodicMessage *table, uint8_t count, uint32_t now_ms) {
  for (uint8_t i = 0; i < count; i++) {
    PeriodicMessage &m = table[i];
    if (m.period_ms == 0 || m.fill == 0) continue;
    if ((int32_t)(now_ms - m.next_due_ms) >= 0) {
      m.next_due_ms += m.period_ms;
      // After a long stall, do not burst to catch up.
      if ((int32_t)(now_ms - m.next_due_ms) >= 0) m.next_due_ms = now_ms + m.period_ms;
      return (int8_t)i;
    }
  }
  return -1;
}

void message_build(const PeriodicMessage &m, uint8_t source, const EngineState &s, Class2Frame &out) {
  out.header = m.header;
  out.target = m.target;
  out.source = source;
  out.len = m.fill ? m.fill(s, out.data) : 0;
  if (out.len > CLASS2_MAX_DATA) out.len = CLASS2_MAX_DATA;
}
