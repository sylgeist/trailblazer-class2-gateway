#!/usr/bin/env python3
"""Decode a capture from class2_log.py and summarize the message streams.

Usage:
    python class2_decode.py ../captures/idle.csv
    python class2_decode.py ../captures/idle.csv --frames ../captures/idle-decoded.csv

The summary groups frames by (header, target, source) and reports how often
each stream repeats. That list is the raw material for docs/message-table.md.
"""
from __future__ import annotations

import argparse
import csv
import sys
from collections import defaultdict

import class2


def load(path: str) -> tuple[list[tuple[float, class2.Frame]], int]:
    """Returns (timestamped frames, number of lines that were not frames)."""
    frames = []
    skipped = 0
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            raw = class2.parse_hex_line(row["raw"])
            frame = class2.decode(raw) if raw is not None else None
            if frame is None:
                skipped += 1
                continue
            frames.append((float(row["t_s"]), frame))
    return frames, skipped


def summarize(frames: list[tuple[float, class2.Frame]]) -> list[dict]:
    """One row per message stream, most frequent first."""
    streams = defaultdict(list)
    for t, frame in frames:
        streams[frame.key].append((t, frame))
    rows = []
    for (header, target, source), items in streams.items():
        times = [t for t, _ in items]
        period_ms = (times[-1] - times[0]) / (len(times) - 1) * 1000 if len(times) > 1 else None
        first = items[0][1]
        rows.append({
            "header": header,
            "priority": first.priority,
            "addressing": "physical" if first.physical else "functional",
            "type": first.msg_type,
            "target": target,
            "source": source,
            "source_name": class2.node_name(source),
            "count": len(items),
            "period_ms": period_ms,
            "crc_errors": sum(1 for _, fr in items if not fr.crc_ok),
            "distinct_payloads": len({fr.data for _, fr in items}),
            "example": first.data.hex(" ").upper(),
        })
    rows.sort(key=lambda r: (-r["count"], r["source"], r["target"]))
    return rows


def print_summary(rows: list[dict], total: int, skipped: int) -> None:
    print(f"{total} frames decoded, {skipped} lines skipped (status text or malformed)\n")
    print(f"{'hdr':>3} {'pri':>3} {'addr':10} {'type':12} {'tgt':>3} {'src':>3} "
          f"{'count':>6} {'period':>9} {'crc!':>4} {'vary':>4}  example / source")
    for r in rows:
        period = f"{r['period_ms']:.0f} ms" if r["period_ms"] is not None else "-"
        print(f"{r['header']:02X}  {r['priority']:>3} {r['addressing']:10} {r['type']:12} "
              f"{r['target']:02X}  {r['source']:02X}  {r['count']:>6} {period:>9} "
              f"{r['crc_errors']:>4} {r['distinct_payloads']:>4}  "
              f"{r['example']}  [{r['source_name']}]")


def write_frames(path: str, frames: list[tuple[float, class2.Frame]]) -> None:
    with open(path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["t_s", "priority", "addressing", "type", "target", "source", "data", "crc_ok"])
        for t, fr in frames:
            writer.writerow([f"{t:.6f}", fr.priority, "physical" if fr.physical else "functional",
                             fr.msg_type, f"{fr.target:02X}", f"{fr.source:02X}",
                             fr.data.hex(" ").upper(), int(fr.crc_ok)])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("capture", help="CSV written by class2_log.py")
    parser.add_argument("--frames", help="also write every decoded frame to this CSV")
    args = parser.parse_args()

    frames, skipped = load(args.capture)
    print_summary(summarize(frames), len(frames), skipped)
    if args.frames:
        write_frames(args.frames, frames)
    return 0


if __name__ == "__main__":
    sys.exit(main())
