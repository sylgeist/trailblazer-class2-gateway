# Trailblazer Class 2 gateway

Firmware and tools for a gateway that lets a 2004 Chevrolet Trailblazer keep its
factory body electronics after the PCM is replaced by a MaxxECU RACE.

```
MaxxECU ──CAN 500k──> MCP2515 ──SPI──> Rugged MEGA-ET ST ──J1850 VPW──> Class 2 bus
                                              │                         (EBCM, HVAC, BCM, airbag)
                                              ├── 128 Hz torque PWM ──> EBCM
                                              └── rear-speed square wave ──> EBCM
```

The gateway reads live engine data from the MaxxECU and takes the factory PCM's
place on the GM Class 2 serial bus (SAE J1850 VPW), so the brake controller,
HVAC module and other factory modules keep seeing the messages they expect.

## Status

| Part | State |
|---|---|
| Bus capture + decode tools (`tools/`) | Written and unit-tested. Not yet run against a real adapter |
| Frame encode/decode, CRC, scheduler, MaxxECU CAN decode | Written and unit-tested on the host |
| MCP2515 receive, torque PWM, speed output, main sketch | Written. **Not yet compiled for AVR or run on the board** |
| Class 2 physical layer (VPW) | Not written. Interface hardware choice is still open |
| Message table | Empty. Filled only from a capture of the stock truck |

As committed, the firmware only listens: every output that can affect the
vehicle is switched off in `firmware/gateway/config.h`.

> **Safety.** On this truck the EBCM provides ABS and rear brake proportioning.
> Nothing in this repo should be connected to the vehicle bus in transmit mode
> until it has been validated on the bench against captured traffic.

## Layout

```
firmware/gateway/   Arduino sketch (gateway.ino) and its modules
test/test_native/   Host-side unit tests for the hardware-independent logic
tools/              Python: capture the bus through an OBD adapter, decode and summarize
docs/               Protocol notes, pins and timers, capture procedure, message table
captures/           Bus logs from the truck
```

## First job: capture the stock bus

This has to happen before the factory PCM comes out. It needs only an
ELM327/STN-based OBD adapter (for example an OBDLink) and a laptop.

```
pip install -r tools/requirements.txt
python tools/class2_log.py <serial-port> captures/idle.csv
python tools/class2_decode.py captures/idle.csv
```

See `docs/capture-procedure.md` for which conditions to record.

## Building the firmware

**Arduino IDE:** open `firmware/gateway/gateway.ino`, select *Arduino Mega or
Mega 2560*, and install the `mcp_can` library (by coryjfowler) from the Library
Manager.

**PlatformIO:** from the repo root:

```
pio run                # build for the Mega
pio run -t upload      # build and upload
pio test -e native     # run the unit tests on this computer
pio device monitor     # serial monitor at 115200
```

Tool tests: `cd tools && python -m unittest`.

## Next steps

1. Capture the bus (above) and fill in `docs/message-table.md`.
2. Scope the PCM-to-EBCM speed wire and the torque PWM lines (`docs/pins-and-timers.md`).
3. Choose the VPW interface (discrete circuit or interpreter IC) and add its
   `Class2Bus` backend.
4. Bench-test transmit against the captured traffic before any vehicle connection.
