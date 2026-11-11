#include "class2_frame.h"
#include "j1850_crc.h"

uint8_t class2_make_header(uint8_t priority, bool physical, uint8_t type) {
  return (uint8_t)(((priority & 0x07) << 5) | 0x08 | (physical ? 0x04 : 0x00) | (type & 0x03));
}

uint8_t class2_encode(const Class2Frame &f, uint8_t *out) {
  if (f.len > CLASS2_MAX_DATA) return 0;
  if (f.header & 0x10) return 0;  // 1-byte header style is not supported
  out[0] = f.header;
  out[1] = f.target;
  out[2] = f.source;
  for (uint8_t i = 0; i < f.len; i++) out[3 + i] = f.data[i];
  uint8_t n = (uint8_t)(3 + f.len);
  out[n] = j1850_crc(out, n);
  return (uint8_t)(n + 1);
}

Class2DecodeResult class2_decode(const uint8_t *bytes, uint8_t n, Class2Frame &out) {
  if (n < 4 || n > CLASS2_MAX_FRAME) return CLASS2_ERR_LENGTH;
  if (bytes[0] & 0x10) return CLASS2_ERR_HEADER_STYLE;
  if (j1850_crc(bytes, (uint8_t)(n - 1)) != bytes[n - 1]) return CLASS2_ERR_CRC;
  out.header = bytes[0];
  out.target = bytes[1];
  out.source = bytes[2];
  out.len = (uint8_t)(n - 4);
  for (uint8_t i = 0; i < out.len; i++) out.data[i] = bytes[3 + i];
  return CLASS2_OK;
}
