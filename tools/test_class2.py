"""Tests for the capture tools:  python -m unittest  (run from tools/)"""
import csv
import os
import tempfile
import unittest

import class2
import class2_decode


class Crc(unittest.TestCase):
    def test_check_value(self):
        self.assertEqual(class2.crc8(b"123456789"), 0x4B)

    def test_known_obd_request(self):
        self.assertEqual(class2.crc8(bytes.fromhex("686AF10100")), 0x17)


class Decode(unittest.TestCase):
    def test_functional_frame(self):
        f = class2.decode(bytes.fromhex("686AF1010017"))
        self.assertEqual((f.priority, f.physical, f.msg_type), (3, False, "function"))
        self.assertEqual((f.target, f.source, f.data), (0x6A, 0xF1, b"\x01\x00"))
        self.assertTrue(f.crc_ok)

    def test_physical_frame_and_bad_crc(self):
        body = bytes.fromhex("6C10F120")
        good = class2.decode(body + bytes([class2.crc8(body)]))
        self.assertTrue(good.physical)
        self.assertEqual(good.msg_type, "node-to-node")
        self.assertTrue(good.crc_ok)
        self.assertFalse(class2.decode(body + b"\x00").crc_ok)

    def test_rejects_non_frames(self):
        self.assertIsNone(class2.decode(b"\x68\x6A\xF1"))          # too short
        self.assertIsNone(class2.decode(bytes(13)))                # too long
        self.assertIsNone(class2.decode(bytes.fromhex("786AF100")))  # 1-byte header style

    def test_node_names(self):
        self.assertEqual(class2.node_name(0x10), "Engine controller")
        self.assertEqual(class2.node_name(0x28), "Brake controller")
        self.assertEqual(class2.node_name(0x99), "Climate control (HVAC)")
        self.assertEqual(class2.node_name(0xF1), "Off-board tester / scan tool")


class ParseLine(unittest.TestCase):
    def test_hex(self):
        self.assertEqual(class2.parse_hex_line("68 6A F1 01 00 17\r"), bytes.fromhex("686AF1010017"))
        self.assertEqual(class2.parse_hex_line("686AF1010017"), bytes.fromhex("686AF1010017"))

    def test_status_text(self):
        for text in ("", "BUFFER FULL", "SEARCHING...", "68 6A F1 <RX ERROR", "NO DATA", "6"):
            self.assertIsNone(class2.parse_hex_line(text), text)


class Summary(unittest.TestCase):
    def test_streams_and_period(self):
        a = bytes.fromhex("8810")  # header, target; source appended below
        rows = [("t_s", "raw")]
        for i in range(5):  # 100 ms stream from node 0x10, payload changes
            body = a + bytes([0x10, 0x01, i])
            rows.append((f"{i * 0.1:.3f}", (body + bytes([class2.crc8(body)])).hex(" ")))
        body = bytes.fromhex("88C299110A")
        rows.append(("0.050", (body + bytes([class2.crc8(body)])).hex(" ")))
        rows.append(("0.060", "BUFFER FULL"))

        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "cap.csv")
            with open(path, "w", newline="") as f:
                csv.writer(f).writerows(rows)
            frames, skipped = class2_decode.load(path)

        self.assertEqual((len(frames), skipped), (6, 1))
        summary = class2_decode.summarize(frames)
        self.assertEqual(len(summary), 2)
        top = summary[0]
        self.assertEqual((top["source"], top["count"], top["distinct_payloads"]), (0x10, 5, 5))
        self.assertAlmostEqual(top["period_ms"], 100.0, places=3)
        self.assertEqual(summary[1]["source_name"], "Climate control (HVAC)")
        self.assertIsNone(summary[1]["period_ms"])


if __name__ == "__main__":
    unittest.main()
