// SAE J1850 CRC-8: polynomial 0x1D, initial value 0xFF, final XOR 0xFF.
#pragma once
#include <stdint.h>

uint8_t j1850_crc(const uint8_t *data, uint8_t len);
