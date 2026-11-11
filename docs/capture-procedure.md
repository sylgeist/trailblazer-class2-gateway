# Capturing the stock Class 2 bus

Do this while the factory PCM is still installed and the truck runs normally.
The captures are the only reference for what the gateway must reproduce.

## Equipment

- ELM327-compatible OBD adapter that supports SAE J1850 VPW. An STN-based
  adapter (OBDLink family) is preferred: clone ELM327 adapters often drop
  frames or overflow in monitor mode.
- Laptop with Python 3.9 or newer and `pip install -r tools/requirements.txt`.
- A USB or Bluetooth-serial connection. Use the highest baud rate the adapter
  supports; if the log shows `BUFFER FULL`, the adapter is being overrun.

## Captures to record

Save each as its own file in `captures/`, a minute or two long unless noted.

| File | Condition |
|---|---|
| `key-on-engine-off.csv` | Ignition on, engine not started. Include the first seconds after key-on |
| `crank-and-idle.csv` | Start logging, then crank; idle until warm-up is under way |
| `idle-warm.csv` | Fully warm idle |
| `ac-cycle.csv` | At idle: AC off, on, off, with 20 s between changes. Note the times |
| `drive.csv` | A short drive: pull away, steady 30 and 50 mph, a firm stop, a turn |
| `key-off.csv` | From running through key-off until the bus goes quiet |
| `pcm-unplugged.csv` | Optional and last: PCM connector unplugged, ignition on. Shows what the other modules send when the PCM is silent, and which faults set |

Write down anything done during a capture (AC button, brake, gear changes) with
the approximate time; it is what lets payload bytes be matched to signals.

## Running

```
python tools/class2_log.py <port> captures/idle-warm.csv
python tools/class2_decode.py captures/idle-warm.csv --frames captures/idle-warm-decoded.csv
```

The summary lists each message stream (header, target, source), how often it
repeats and whether its payload changes. Streams with source `10` are the ones
the gateway has to take over.

## Also measure with a scope or logic analyzer

- PCM to EBCM vehicle speed wire: voltage levels, pulses per mile, duty cycle,
  behaviour at a stop.
- Delivered and requested torque PWM lines: frequency, and duty cycle at idle,
  light throttle and wide-open throttle.
- The Class 2 wire itself, once: confirms the 0 to 7 V levels for the interface design.
