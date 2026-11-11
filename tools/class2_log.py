#!/usr/bin/env python3
"""Capture every frame on the Class 2 bus through an ELM327/STN OBD adapter.

The adapter is put in J1850 VPW monitor-all mode with headers shown, and each
line it prints is written to a CSV with a timestamp:

    t_s,raw
    0.000000,68 6A F1 01 00 17

Usage:
    python class2_log.py /dev/ttyUSB0 ../captures/idle.csv
    python class2_log.py COM5 ../captures/key-on.csv --baud 115200

Stop with Ctrl-C. Run class2_decode.py on the result.
"""
from __future__ import annotations

import argparse
import csv
import sys
import time

import serial  # pyserial

INIT_COMMANDS = [
    "ATZ",    # reset
    "ATE0",   # echo off
    "ATL0",   # no linefeeds
    "ATS1",   # spaces between bytes
    "ATH1",   # show header bytes (and the CRC)
    "ATSP2",  # protocol 2 = SAE J1850 VPW
]


def command(port: serial.Serial, text: str, timeout_s: float = 3.0) -> str:
    """Send one AT command and return the reply up to the '>' prompt."""
    port.reset_input_buffer()
    port.write((text + "\r").encode("ascii"))
    deadline = time.monotonic() + timeout_s
    reply = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(64)
        reply += chunk
        if b">" in chunk:
            break
    return reply.decode("ascii", errors="replace").replace(">", "").strip()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("port", help="serial port of the OBD adapter")
    parser.add_argument("output", help="CSV file to write")
    parser.add_argument("--baud", type=int, default=115200, help="adapter baud rate (default 115200)")
    args = parser.parse_args()

    with serial.Serial(args.port, args.baud, timeout=0.1) as port, \
            open(args.output, "w", newline="") as out:
        for cmd in INIT_COMMANDS:
            reply = command(port, cmd)
            print(f"{cmd:6s} -> {reply!r}", file=sys.stderr)

        writer = csv.writer(out)
        writer.writerow(["t_s", "raw"])
        port.write(b"ATMA\r")  # monitor all
        print("Monitoring. Ctrl-C to stop.", file=sys.stderr)

        start = time.monotonic()
        pending = bytearray()
        lines = 0
        try:
            while True:
                pending += port.read(256)
                while b"\r" in pending:
                    line, _, rest = pending.partition(b"\r")
                    pending = bytearray(rest)
                    text = line.decode("ascii", errors="replace").strip()
                    if not text:
                        continue
                    writer.writerow([f"{time.monotonic() - start:.6f}", text])
                    lines += 1
                    if "BUFFER FULL" in text:
                        # The adapter leaves monitor mode when its buffer overflows.
                        print("Adapter buffer overflowed; restarting monitor. "
                              "Use a higher --baud if this repeats.", file=sys.stderr)
                        port.write(b"ATMA\r")
        except KeyboardInterrupt:
            pass
        finally:
            port.write(b"\r")  # any character ends monitor mode
            print(f"Wrote {lines} lines to {args.output}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
