#include <Arduino.h>
#include "class2_bus.h"

bool Class2BusNull::begin() { return true; }

bool Class2BusNull::send(const uint8_t *bytes, uint8_t len) {
  Serial.print(F("C2 TX (null bus):"));
  for (uint8_t i = 0; i < len; i++) {
    Serial.print(' ');
    if (bytes[i] < 0x10) Serial.print('0');
    Serial.print(bytes[i], HEX);
  }
  Serial.println();
  return true;
}

uint8_t Class2BusNull::receive(uint8_t *, uint8_t) { return 0; }
