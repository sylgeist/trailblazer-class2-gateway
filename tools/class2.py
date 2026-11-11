"""GM Class 2 (SAE J1850 VPW) frame helpers shared by the capture tools.

Header layout and node address ranges follow docs/protocol.md.
"""
from __future__ import annotations

import re
from dataclasses import dataclass

MAX_FRAME = 12  # 3 header + 8 data + 1 CRC

_FUNCTIONAL_TYPES = {0: "function", 1: "broadcast", 2: "query", 3: "read"}
_PHYSICAL_TYPES = {0: "node-to-node", 1: "reserved", 2: "reserved", 3: "reserved"}

# (first, last, name) - SAE J2178 physical node address ranges.
_NODE_RANGES = [
    (0x00, 0x0F, "Powertrain (integration)"),
    (0x10, 0x17, "Engine controller"),
    (0x18, 0x1F, "Transmission controller"),
    (0x20, 0x27, "Chassis (integration)"),
    (0x28, 0x2F, "Brake controller"),
    (0x30, 0x37, "Steering controller"),
    (0x38, 0x3F, "Suspension controller"),
    (0x40, 0x57, "Body (integration)"),
    (0x58, 0x5F, "Restraints"),
    (0x60, 0x6F, "Driver information / displays"),
    (0x70, 0x7F, "Lighting"),
    (0x80, 0x8F, "Entertainment"),
    (0x90, 0x97, "Personal communications"),
    (0x98, 0x9F, "Climate control (HVAC)"),
    (0xA0, 0xBF, "Convenience (doors, seats, windows)"),
    (0xC0, 0xC7, "Security"),
    (0xC8, 0xCB, "EV energy transfer"),
    (0xCC, 0xCF, "Future expansion"),
    (0xD0, 0xEF, "Manufacturer specific"),
    (0xF0, 0xFD, "Off-board tester / scan tool"),
    (0xFE, 0xFE, "All nodes"),
    (0xFF, 0xFF, "Null node"),
]


def crc8(data: bytes) -> int:
    """CRC-8/SAE-J1850: polynomial 0x1D, init 0xFF, final XOR 0xFF."""
    crc = 0xFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1D) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc ^ 0xFF


def node_name(addr: int) -> str:
    for first, last, name in _NODE_RANGES:
        if first <= addr <= last:
            return name
    return "Unknown"


@dataclass(frozen=True)
class Frame:
    raw: bytes
    priority: int
    physical: bool
    msg_type: str
    target: int
    source: int
    data: bytes
    crc_ok: bool

    @property
    def key(self) -> tuple[int, int, int]:
        """Identity of a message stream: header byte, target, source."""
        return (self.raw[0], self.target, self.source)


def decode(raw: bytes) -> Frame | None:
    """Decode frame bytes that include the trailing CRC.

    Returns None for anything that cannot be a 3-byte-header frame.
    """
    if not 4 <= len(raw) <= MAX_FRAME:
        return None
    header = raw[0]
    if header & 0x10:  # 1-byte header style
        return None
    physical = bool(header & 0x04)
    types = _PHYSICAL_TYPES if physical else _FUNCTIONAL_TYPES
    return Frame(
        raw=bytes(raw),
        priority=header >> 5,
        physical=physical,
        msg_type=types[header & 0x03],
        target=raw[1],
        source=raw[2],
        data=bytes(raw[3:-1]),
        crc_ok=crc8(raw[:-1]) == raw[-1],
    )


_HEX_LINE = re.compile(r"^(?:[0-9A-Fa-f]{2})(?:\s*[0-9A-Fa-f]{2})*$")


def parse_hex_line(text: str) -> bytes | None:
    """Parse an adapter output line such as '68 6A F1 01 00 17'.

    Returns None for status text ('BUFFER FULL', '<RX ERROR', 'SEARCHING...')
    and for lines flagged with an error marker.
    """
    text = text.strip()
    if not text or not _HEX_LINE.match(text):
        return None
    return bytes.fromhex(text.replace(" ", ""))
