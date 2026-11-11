# Protocol notes

## GM Class 2 = SAE J1850 VPW

- Single wire, 10.4 kbit/s average, variable pulse width.
- The bus idles low and is driven high to about 7 V.
- Bits are pulses of 64 µs or 128 µs, alternating high and low, after a 200 µs
  start-of-frame. This is **not** UART framing: a hardware UART cannot send or
  receive it at any baud rate.
- LIN transceivers (for example TLIN1021A-Q1) do not work: LIN is UART-framed,
  idles near battery voltage and is dominant low.
- Single-wire CAN transceivers (for example TH8056) do not work: that is GMLAN,
  a later GM bus.
- On the OBD-II connector the bus is pin 2.

### Frame

```
byte 0   bits 7-5  priority (0 = highest, 7 = lowest)
         bit 4     header style (0 = 3-byte header, which GM uses)
         bit 3     in-frame response (1 = not allowed, which GM uses)
         bit 2     addressing (1 = physical, 0 = functional)
         bits 1-0  message type
byte 1   target: physical node address, or functional ID
byte 2   source: physical node address
bytes 3+ data, 0 to 8 bytes
last     CRC-8: polynomial 0x1D, initial 0xFF, final XOR 0xFF
```

A frame is at most 12 bytes. Check values used in the tests: the ASCII string
`123456789` gives CRC `4B`; the OBD-II request `68 6A F1 01 00` gives `17`.

Message type (bits 1-0):

| Addressing | 0 | 1 | 2 | 3 |
|---|---|---|---|---|
| Functional | function | broadcast | query | read |
| Physical | node-to-node | reserved | reserved | reserved |

### Physical node addresses

| Range | Module group |
|---|---|
| `10-17` | Engine controllers (the factory PCM is `10`) |
| `18-1F` | Transmission controllers |
| `28-2F` | Brake controllers |
| `58-5F` | Restraints |
| `60-6F` | Driver information / displays |
| `98-9F` | Climate control (HVAC) |
| `A0-BF` | Convenience (doors, seats, windows) |
| `C0-C7` | Security |
| `F0-FD` | Off-board testers / scan tools (`F1` is typical) |
| `FE` | All nodes |

The full list is in `tools/class2.py`. Source: the *VPW Communication Protocol*
reference kept with the build project.

## MaxxECU default CAN protocol

500 kbit/s, 11-bit IDs, 8-byte frames, little-endian signed 16-bit fields.
Fields the gateway reads:

| ID | Offset | Signal | Scale | Rate |
|---|---|---|---|---|
| `0x520` | 0 | RPM | 1 rpm | 50 Hz |
| `0x520` | 2 | Throttle position | 0.1 % | 50 Hz |
| `0x520` | 4 | MAP | 0.1 kPa | 50 Hz |
| `0x522` | 6 | Vehicle speed | 0.1 km/h | 50 Hz |
| `0x530` | 0 | Battery voltage | 0.01 V | 10 Hz |
| `0x530` | 4 | Intake air temperature | 0.1 °C | 10 Hz |
| `0x530` | 6 | Coolant temperature | 0.1 °C | 10 Hz |
| `0x536` | 0 | Gear | 1 | 10 Hz |
| `0x536` | 4 | Oil pressure | 0.1 kPa | 10 Hz |

Source: <https://maxxecu.com/webhelp/can-default_maxxecu_protocol.html>
(protocol v1.3). AC request and AC high-side pressure are not in the default
protocol; they will need MaxxECU user CAN channels (`0x538`/`0x539`) or a
custom CAN output.

The gateway runs the MCP2515 in listen-only mode: it never transmits or
acknowledges on the MaxxECU bus.
