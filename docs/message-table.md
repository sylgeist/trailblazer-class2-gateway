# Class 2 message table

The gateway may only transmit messages that were observed from the factory PCM
(source `10`) in a capture of this truck. Record them here first, then add the
matching entry to `firmware/gateway/message_table.cpp`.

## Messages to reproduce

| Header | Target | Period | Data layout | Driven by | Needed by | Evidence |
|---|---|---|---|---|---|---|
| | | | | | | |

*Evidence* is the capture file and what was done to confirm the meaning of each
byte (for example "byte 4 rises with coolant temperature in `crank-and-idle.csv`").

## Messages to listen for

| Header | Target | Source | Meaning | Gateway action | Evidence |
|---|---|---|---|---|---|
| | | | | | |

## Leads to confirm (not verified)

- HVAC: AC high-side pressure is reported as functional ID pair `C2`/`C3`,
  secondary ID `11`, value in kPa, from the PCM to the HVAC module. From
  community notes, not from this truck.
- HVAC: an AC request from the HVAC module to the PCM, and coolant temperature
  from the PCM to the HVAC module. IDs unknown.
- EBCM, airbag module, BCM: which PCM messages each needs to stay fault-free
  is unknown. The `pcm-unplugged.csv` capture and a scan tool will show it.
