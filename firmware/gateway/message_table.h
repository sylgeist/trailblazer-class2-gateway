// Periodic Class 2 messages the gateway broadcasts in place of the factory PCM.
//
// The table in message_table.cpp is EMPTY on purpose. Entries are added only
// from messages seen in a capture of the stock truck (docs/message-table.md);
// nothing here is guessed.
#pragma once
#include <stdint.h>
#include "class2_frame.h"
#include "engine_state.h"

// Writes the data bytes for one message from live engine data.
// Returns the number of data bytes written (0-8).
typedef uint8_t (*Class2FillFn)(const EngineState &s, uint8_t *data);

struct PeriodicMessage {
  uint8_t      header;      // byte 0, see class2_frame.h
  uint8_t      target;      // functional ID or physical address
  uint16_t     period_ms;
  Class2FillFn fill;
  uint32_t     next_due_ms; // scheduler state
};

extern PeriodicMessage g_messages[];
extern const uint8_t   g_message_count;

// Returns the index of a message that is due at `now_ms` and advances its
// schedule, or -1 if nothing is due. Safe across millis() rollover.
int8_t scheduler_poll(PeriodicMessage *table, uint8_t count, uint32_t now_ms);

// Builds the frame for table entry `m` with the gateway as source.
void message_build(const PeriodicMessage &m, uint8_t source, const EngineState &s, Class2Frame &out);
