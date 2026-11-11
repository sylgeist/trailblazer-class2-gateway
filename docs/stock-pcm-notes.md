# Stock PCM notes

Read from the truck's stock calibration in HP Tuners VCM Editor (November 2026).
Values were transcribed from screenshots. A compare against the owner's
modified tune showed no differences in DTC settings or speedometer values, and
only one torque-management change (ETC TPS Max Delta Positive, set to 100 %),
so everything below is the factory calibration. Anything marked *inferred* is a
reading of the data, not something the editor states.

## Module

- Hardware: P10 ECM, GM 4.2L I6
- Operating system: 12597031

## Modules the PCM monitors on Class 2

The calibration's U-code list shows which "lost communication" faults the PCM
has enabled. On GM Class 2, a `U1xxx` loss-of-communication code is 1000 plus
the module's node address in decimal, so each code also gives an address.

| DTC | Editor description | Node address | Enabled in PCM |
|---|---|---|---|
| U1026 | Loss of ATC Class 2 communication (transfer case control) | `1A` | Yes |
| U1040 | Loss of Class 2 communications with ABS | `28` | No |
| U1041 | Loss of EBCM communication | `29` | Yes |
| U1064 | Loss of communications with DIM (dash integration module) | `40` | Yes |
| U1096 | Loss of communications with IPC (instrument cluster) | `60` | Yes |
| U1152 | Loss of ACM serial data communication | `98` | Yes |

"Enabled" means the editor shows *No MIL Light* (fault stored, lamp off);
"No" means *No Error Reported*.

*Inferred:*

- `98` is in the climate-control address range, so "ACM" is most likely the
  HVAC control module.
- The PCM itself is node `10`, so the fault other modules set when the PCM
  goes silent should be U1016. Confirm with a scan tool during the
  `pcm-unplugged` capture.
- These five modules send something the PCM listens for. Expect sources
  `1A`, `29`, `40`, `60` and `98` in the capture, plus `10` for the PCM.

Truck-specific note: the transfer case control module (`1A`) was removed when
the NV120 AWD transfer case went in. The only visible effect was the "Service
4WD" lamp (its LED has been removed). So `1A` will not appear in captures, and
the PCM should be holding a stored U1026. Reading the PCM's codes with a scan
tool is a quick check of the address-from-code rule above.

Also enabled: U1300 (Class 2 short to ground) and U1301 (Class 2 short to
battery). U1000 (Class 2 communication malfunction) is not enabled.

## Speedometer calibration

| Setting | Value |
|---|---|
| Drive sprocket ratio | 1.000 |
| Trans revolutions per mile | 4,244.8013 |
| VSS pulses per mile | 104,928 |
| VSS pulses per mile (Bus) | 107,448 |
| VSS trans revs per mile | 4,221.6984 |

- These are the PCM's speed **input** scalings. They do not give the pulse
  rate of the hardwired speed signal the PCM sends to the EBCM; that still has
  to be measured on the wire.
- *Inferred:* the "(Bus)" figure is the scaling for speed or distance the PCM
  reports over Class 2, which suggests a speed-related PCM broadcast to look
  for in the capture.
- Unexplained: 4,244.8 trans revolutions per mile is well above what a 3.73
  axle and a normal tire size give (roughly 2,500). It is the factory value,
  not a later rescale, so the field must count something other than
  driveshaft turns. Do not build on this figure until it is understood.

## Security

- VATS control: enabled
- VATS fuel cut: enabled

The factory PCM waits for a theft-deterrent password over Class 2 before it
allows fuel. With the MaxxECU running the engine this no longer gates
starting. Watch the capture for that exchange at key-on and check whether the
security lamp behaves differently once the PCM is gone.

## Air conditioning

The only A/C table in the calibration:

| | AC disable | AC re-enable |
|---|---|---|
| In gear | 8,000 | 7,500 |
| P/N | 8,000 | 7,500 |

Units are not shown; *inferred* to be engine speed in rpm, in which case the
cut-out is above the engine's rev range and never acts. Pressure and coolant
cut-outs are not exposed in this calibration.

## Torque management

Sections present: maximum torque (vs. gear, 4WD low, 4WD high), ETC throttle
limits, and spark retard vs. torque reduction.

Spark retard vs. torque reduction. Rows are requested torque reduction (%),
columns are engine speed (rpm). Cell units are not shown; presumably degrees.

| % | 400 | 800 | 1200 | 1600 | 2000 | 2400 | 2800 | 3200 | 3600 | 4000 | 4400 | 4800 | 5200 | 5600 | 6000 | 6400 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| 10 | 13 | 9 | 7 | 8 | 9 | 9 | 10 | 10 | 10 | 11 | 10 | 10 | 11 | 11 | 12 | 16 |
| 20 | 20 | 15 | 13 | 13 | 14 | 14 | 16 | 16 | 16 | 16 | 16 | 16 | 16 | 16 | 19 | 22 |
| 30 | 24 | 19 | 16 | 17 | 17 | 19 | 21 | 21 | 21 | 21 | 21 | 20 | 21 | 20 | 25 | 27 |
| 40 | 26 | 21 | 18 | 19 | 17 | 21 | 24 | 24 | 26 | 26 | 24 | 24 | 24 | 25 | 28 | 30 |
| 50 | 28 | 24 | 21 | 22 | 21 | 22 | 26 | 27 | 29 | 29 | 27 | 28 | 27 | 28 | 31 | 33 |
| 60 | 31 | 27 | 26 | 25 | 23 | 27 | 29 | 30 | 33 | 32 | 30 | 32 | 31 | 32 | 34 | 36 |
| 70 | 33 | 30 | 29 | 28 | 26 | 31 | 33 | 33 | 36 | 34 | 33 | 36 | 35 | 36 | 38 | 39 |
| 80 | 35 | 33 | 32 | 31 | 31 | 34 | 36 | 36 | 39 | 36 | 36 | 41 | 39 | 39 | 42 | 44 |
| 90 | 36 | 34 | 34 | 34 | 33 | 38 | 39 | 39 | 42 | 39 | 39 | 46 | 44 | 44 | 48 | 50 |
| 100 | 49 | 48 | 48 | 48 | 60 | 42 | 42 | 42 | 42 | 39 | 39 | 46 | 44 | 44 | 48 | 50 |

This is how the factory PCM turned a torque-reduction request into spark
retard. It is a reference for how hard the MaxxECU should respond when the
EBCM asks for less torque, once that request signal is characterized.
