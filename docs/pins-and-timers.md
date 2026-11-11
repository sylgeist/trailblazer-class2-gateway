# Pins and timers (Rugged MEGA-ET ST, ATmega2560)

## Pins

| Pin | Function | Notes |
|---|---|---|
| 2 (INT4) | MCP2515 interrupt | Active low |
| 3 (INT5) | Class 2 receive | Discrete VPW interface only |
| 4 | Class 2 transmit drive | Discrete VPW interface only |
| 5 (OC3A) | Rear-speed square wave | Needs an external high-side stage |
| 11 (OC1A) | Delivered torque PWM, 128 Hz | |
| 12 (OC1B) | Requested torque PWM, 128 Hz | |
| 13 | Status LED | Toggles once a second |
| 18/19 (Serial1) | Reserved | Interpreter-IC VPW option |
| 50-52 | SPI | MCP2515 |
| 53 (SS) | MCP2515 chip select | |

## Timers

| Timer | Use | Setting |
|---|---|---|
| Timer0 | `millis()` | Arduino default |
| Timer1 | Torque PWM | Fast PWM, TOP 15624, prescaler 8: exactly 128 Hz |
| Timer2 | Reserved for VPW | The J1850VPWCore library uses it for bus-idle timing |
| Timer3 | Rear-speed output | CTC, toggle OC3A, prescaler 256; 0.5 Hz to several kHz |
| Timer4 | Reserved for VPW | The J1850VPWCore library uses it for transmit timing |
| Timer5 | Free | |

If the VPW layer cannot share the processor cleanly with the two outputs, the
fallback is a small dedicated microcontroller for VPW that talks to the Mega.

## Hardware items still open

- **FeatherWing logic level.** Confirm how the CAN FeatherWing #5709 powers its
  MCP2515. If the controller runs at 3.3 V, the Mega's 5 V SPI lines must not
  be connected directly; either power the logic side at 5 V if the board
  allows it, or add a level shifter.
- **MCP2515 crystal.** `config.h` assumes 16 MHz. Read the marking on the board.
- **CAN termination.** The FeatherWing's 120 Ω jumper is closed by default.
  Cut it unless the gateway ends up at a physical end of the bus.
- **Class 2 interface.** Open choice between a discrete circuit (divider or
  comparator for receive, transistor stage from a 7-8 V rail for transmit) and
  a UART-to-VPW interpreter IC.
- **Rear-speed output stage.** The factory PCM switches this line high into a
  pull-down inside the EBCM. Measure the stock signal first: high level,
  pulses per mile (`SPEED_PULSES_PER_MILE` is an unconfirmed 4000), duty cycle
  and behaviour at a stop.
- **Torque PWM meaning.** Frequency is set; what each duty cycle represents
  has to be measured on the stock truck. Until then the firmware holds both
  lines low.
