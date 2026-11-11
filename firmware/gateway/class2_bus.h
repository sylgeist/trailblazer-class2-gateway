// Transport for Class 2 frames. The gateway logic talks only to this interface,
// so the physical layer can be swapped without touching anything else:
//
//   Class2BusNull        - no hardware; prints outgoing frames to Serial (bench)
//   (to add) discrete    - bit-banged VPW on PIN_VPW_RX / PIN_VPW_TX
//   (to add) interpreter - UART-to-VPW IC on Serial1
#pragma once
#include <stdint.h>

class Class2Bus {
public:
  virtual ~Class2Bus() {}
  virtual bool begin() = 0;
  // Sends complete frame bytes, CRC included. Returns false if the bus was busy
  // or arbitration was lost; the caller retries on the next scheduler pass.
  virtual bool send(const uint8_t *bytes, uint8_t len) = 0;
  // Copies one received frame (CRC included) into `buf`. Returns its length, 0 if none.
  virtual uint8_t receive(uint8_t *buf, uint8_t max_len) = 0;
};

class Class2BusNull : public Class2Bus {
public:
  bool begin() override;
  bool send(const uint8_t *bytes, uint8_t len) override;
  uint8_t receive(uint8_t *buf, uint8_t max_len) override;
};
