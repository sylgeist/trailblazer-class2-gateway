// Host-side tests for the hardware-independent gateway logic.
//   pio test -e native
#include <unity.h>
#include <string.h>
#include "j1850_crc.h"
#include "class2_frame.h"
#include "message_table.h"
#include "maxx_decode.h"
#include "speed_math.h"

void setUp() {}
void tearDown() {}

static void test_crc_check_value() {
  // Standard check value for CRC-8/SAE-J1850 over the ASCII string "123456789".
  TEST_ASSERT_EQUAL_HEX8(0x4B, j1850_crc((const uint8_t *)"123456789", 9));
}

static void test_crc_known_obd_frame() {
  // The legislated OBD-II VPW request "68 6A F1 01 00" carries CRC 0x17.
  const uint8_t req[] = {0x68, 0x6A, 0xF1, 0x01, 0x00};
  TEST_ASSERT_EQUAL_HEX8(0x17, j1850_crc(req, sizeof(req)));
}

static void test_decode_known_frame() {
  const uint8_t raw[] = {0x68, 0x6A, 0xF1, 0x01, 0x00, 0x17};
  Class2Frame f;
  TEST_ASSERT_EQUAL(CLASS2_OK, class2_decode(raw, sizeof(raw), f));
  TEST_ASSERT_EQUAL_UINT8(3, class2_priority(f));
  TEST_ASSERT_FALSE(class2_is_physical(f));
  TEST_ASSERT_EQUAL_HEX8(0x6A, f.target);
  TEST_ASSERT_EQUAL_HEX8(0xF1, f.source);
  TEST_ASSERT_EQUAL_UINT8(2, f.len);
  TEST_ASSERT_EQUAL_HEX8(0x01, f.data[0]);
}

static void test_decode_rejects_bad_input() {
  Class2Frame f;
  const uint8_t bad_crc[] = {0x68, 0x6A, 0xF1, 0x01, 0x00, 0x18};
  TEST_ASSERT_EQUAL(CLASS2_ERR_CRC, class2_decode(bad_crc, sizeof(bad_crc), f));
  const uint8_t too_short[] = {0x68, 0x6A, 0xF1};
  TEST_ASSERT_EQUAL(CLASS2_ERR_LENGTH, class2_decode(too_short, sizeof(too_short), f));
  const uint8_t one_byte_hdr[] = {0x78, 0x6A, 0xF1, 0x00};
  TEST_ASSERT_EQUAL(CLASS2_ERR_HEADER_STYLE, class2_decode(one_byte_hdr, sizeof(one_byte_hdr), f));
}

static void test_encode_round_trip() {
  Class2Frame f;
  f.header = class2_make_header(3, false, 0);
  TEST_ASSERT_EQUAL_HEX8(0x68, f.header);
  f.target = 0x6A; f.source = 0xF1; f.len = 2; f.data[0] = 0x01; f.data[1] = 0x00;
  uint8_t out[CLASS2_MAX_FRAME];
  TEST_ASSERT_EQUAL_UINT8(6, class2_encode(f, out));
  const uint8_t expect[] = {0x68, 0x6A, 0xF1, 0x01, 0x00, 0x17};
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 6);

  f.len = 9;
  TEST_ASSERT_EQUAL_UINT8(0, class2_encode(f, out));
}

static uint8_t fill_one(const EngineState &, uint8_t *d) { d[0] = 0xAA; return 1; }

static void test_scheduler_period_and_rollover() {
  PeriodicMessage t[2] = {
    { 0x88, 0x10, 100, fill_one, 0 },
    { 0x88, 0x20, 0,   fill_one, 0 },   // period 0 = disabled
  };
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 0));
  TEST_ASSERT_EQUAL_INT8(-1, scheduler_poll(t, 2, 50));
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 100));
  TEST_ASSERT_EQUAL_INT8(-1, scheduler_poll(t, 2, 100));

  // After a long stall the schedule restarts from "now" without a burst.
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 5000));
  TEST_ASSERT_EQUAL_INT8(-1, scheduler_poll(t, 2, 5001));
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 5100));

  // Across millis() rollover.
  t[0].next_due_ms = 0xFFFFFFF0UL;
  TEST_ASSERT_EQUAL_INT8(-1, scheduler_poll(t, 2, 0xFFFFFFE0UL));
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 0xFFFFFFF5UL));
  TEST_ASSERT_EQUAL_INT8(-1, scheduler_poll(t, 2, 0x00000010UL));
  TEST_ASSERT_EQUAL_INT8(0, scheduler_poll(t, 2, 0x00000060UL));
}

static void test_message_table_ships_empty() {
  TEST_ASSERT_EQUAL_UINT8(0, g_message_count);
}

static void test_message_build() {
  PeriodicMessage m = { 0x88, 0x10, 100, fill_one, 0 };
  EngineState s; memset(&s, 0, sizeof(s));
  Class2Frame f;
  message_build(m, 0x10, s, f);
  TEST_ASSERT_EQUAL_HEX8(0x10, f.source);
  TEST_ASSERT_EQUAL_UINT8(1, f.len);
  TEST_ASSERT_EQUAL_HEX8(0xAA, f.data[0]);
}

static void test_maxx_decode() {
  EngineState s; memset(&s, 0, sizeof(s));
  // 0x520: rpm 2500, throttle 12.3 %, MAP 101.3 kPa, lambda 1.000 (little-endian)
  const uint8_t f520[8] = {0xC4, 0x09, 0x7B, 0x00, 0xF5, 0x03, 0xE8, 0x03};
  TEST_ASSERT_TRUE(maxx_decode(0x520, f520, 8, 1000, s));
  TEST_ASSERT_EQUAL_INT16(2500, s.rpm);
  TEST_ASSERT_EQUAL_INT16(123, s.throttle_x10);
  TEST_ASSERT_EQUAL_INT16(1013, s.map_kpa_x10);
  // 0x530: battery 13.80 V, baro, IAT 25.0 C, coolant -5.0 C (negative value)
  const uint8_t f530[8] = {0x64, 0x05, 0x00, 0x00, 0xFA, 0x00, 0xCE, 0xFF};
  TEST_ASSERT_TRUE(maxx_decode(0x530, f530, 8, 1010, s));
  TEST_ASSERT_EQUAL_INT16(1380, s.battery_v_x100);
  TEST_ASSERT_EQUAL_INT16(250, s.iat_c_x10);
  TEST_ASSERT_EQUAL_INT16(-50, s.coolant_c_x10);

  TEST_ASSERT_FALSE(maxx_decode(0x7FF, f520, 8, 1010, s));   // unknown ID
  TEST_ASSERT_FALSE(maxx_decode(0x520, f520, 4, 1010, s));   // short frame
}

static void test_maxx_freshness() {
  EngineState s; memset(&s, 0, sizeof(s));
  TEST_ASSERT_FALSE(maxx_data_fresh(s, 100, 500));           // nothing received yet
  s.last_fast_ms = 1000; s.last_slow_ms = 1000;
  TEST_ASSERT_TRUE(maxx_data_fresh(s, 1400, 500));
  TEST_ASSERT_FALSE(maxx_data_fresh(s, 1600, 500));          // fast frame stale
}

static void test_speed_math() {
  // 100 mph = 160.9 km/h at 4000 pulses/mile is 111.1 Hz.
  uint32_t mhz = speed_freq_mhz(1609, 4000);
  TEST_ASSERT_UINT32_WITHIN(200, 111100, mhz);
  // Toggling output: half period = 62500 / (2 * 111.1) = 281 ticks.
  TEST_ASSERT_UINT16_WITHIN(1, 280, speed_ocr(mhz));
  // Stopped or crawling below 0.5 Hz switches the output off.
  TEST_ASSERT_EQUAL_UINT32(0, speed_freq_mhz(0, 4000));
  TEST_ASSERT_EQUAL_UINT32(0, speed_freq_mhz(-5, 4000));
  TEST_ASSERT_EQUAL_UINT16(0, speed_ocr(499));
  TEST_ASSERT_EQUAL_UINT16(62499, speed_ocr(500));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_crc_check_value);
  RUN_TEST(test_crc_known_obd_frame);
  RUN_TEST(test_decode_known_frame);
  RUN_TEST(test_decode_rejects_bad_input);
  RUN_TEST(test_encode_round_trip);
  RUN_TEST(test_scheduler_period_and_rollover);
  RUN_TEST(test_message_table_ships_empty);
  RUN_TEST(test_message_build);
  RUN_TEST(test_maxx_decode);
  RUN_TEST(test_maxx_freshness);
  RUN_TEST(test_speed_math);
  return UNITY_END();
}
