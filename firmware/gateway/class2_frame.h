// GM Class 2 (SAE J1850 VPW) frame with the 3-byte header.
//
//   byte 0  bits 7-5 priority (0 = highest, 7 = lowest)
//           bit 4    header style (0 = 3-byte header)
//           bit 3    in-frame response (1 = not allowed, as GM uses)
//           bit 2    addressing (1 = physical, 0 = functional)
//           bits 1-0 message type
//   byte 1  target (physical node address or functional ID)
//   byte 2  source (physical node address)
//   bytes 3..n  data (0-8 bytes)
//   last byte   CRC
#pragma once
#include <stdint.h>

#define CLASS2_MAX_DATA   8
#define CLASS2_MAX_FRAME  12   // 3 header + 8 data + 1 CRC

struct Class2Frame {
  uint8_t header;                  // raw byte 0
  uint8_t target;
  uint8_t source;
  uint8_t data[CLASS2_MAX_DATA];
  uint8_t len;                     // number of data bytes
};

inline uint8_t class2_priority(const Class2Frame &f)    { return (uint8_t)(f.header >> 5); }
inline bool    class2_is_physical(const Class2Frame &f) { return (f.header & 0x04) != 0; }
inline uint8_t class2_type(const Class2Frame &f)        { return (uint8_t)(f.header & 0x03); }

// Build byte 0 for a GM-style frame (3-byte header, no in-frame response).
uint8_t class2_make_header(uint8_t priority, bool physical, uint8_t type);

// Serialize a frame and append the CRC. `out` must hold CLASS2_MAX_FRAME bytes.
// Returns the number of bytes written, or 0 if the frame is invalid.
uint8_t class2_encode(const Class2Frame &f, uint8_t *out);

enum Class2DecodeResult {
  CLASS2_OK = 0,
  CLASS2_ERR_LENGTH,        // shorter than header + CRC, or longer than 12 bytes
  CLASS2_ERR_HEADER_STYLE,  // 1-byte header, which this gateway does not handle
  CLASS2_ERR_CRC
};

// Parse raw bytes (including the trailing CRC) into a frame.
Class2DecodeResult class2_decode(const uint8_t *bytes, uint8_t n, Class2Frame &out);
