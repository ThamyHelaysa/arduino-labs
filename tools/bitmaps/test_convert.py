#!/usr/bin/env python3
"""Self-check for convert.py: python3 tools/bitmaps/test_convert.py"""

import struct
import tempfile
import unittest
import zlib
from pathlib import Path

from convert import SIGNATURE, convert, pack, paeth, read_png, scale_up

W, K, R = (255, 255, 255), (0, 0, 0), (255, 0, 0)
IMAGE = [  # 9 wide, so each row spills into a second byte
    [K, W, W, W, W, W, W, W, R],
    [W, R, K, K, W, W, W, K, K],
    [R, R, R, W, W, W, W, W, W],
]


def chunk(kind, body):
    return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body))


def write_png(path, rows, filter_kind):
    """Encodes RGB rows with one PNG filter applied to every row."""
    raw, previous = b"", bytes(len(rows[0]) * 3)
    for row in rows:
        line = bytes(c for pixel in row for c in pixel)
        out = bytearray()
        for i, value in enumerate(line):
            left = line[i - 3] if i >= 3 else 0
            up_left = previous[i - 3] if i >= 3 else 0
            predictor = (0, left, previous[i], (left + previous[i]) // 2, paeth(left, previous[i], up_left))[filter_kind]
            out.append((value - predictor) & 0xFF)
        raw += bytes([filter_kind]) + out
        previous = line
    header = struct.pack(">IIBBBBB", len(rows[0]), len(rows), 8, 2, 0, 0, 0)
    Path(path).write_bytes(SIGNATURE + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


class ConvertTest(unittest.TestCase):
    def test_every_filter_decodes_to_the_same_pixels(self):
        with tempfile.TemporaryDirectory() as folder:
            for filter_kind in range(5):
                path = Path(folder) / f"f{filter_kind}.png"
                write_png(path, IMAGE, filter_kind)
                _, _, rows = read_png(path)
                self.assertEqual([[p[:3] for p in row] for row in rows], IMAGE, f"filter {filter_kind}")

    def test_layers_pack_msb_first_with_row_padding(self):
        rows = [[p + (255,) for p in row] for row in IMAGE]
        self.assertEqual(pack(rows, "black"), [0x80, 0x00, 0x31, 0x80, 0x00, 0x00])
        self.assertEqual(pack(rows, "red"), [0x00, 0x80, 0x40, 0x00, 0xE0, 0x00])

    def test_scale_repeats_each_pixel(self):
        self.assertEqual(scale_up([[1, 2]], 2), [[1, 1, 2, 2], [1, 1, 2, 2]])

    def test_rejects_mixed_sizes(self):
        with tempfile.TemporaryDirectory() as folder:
            write_png(Path(folder) / "a.png", IMAGE, 0)
            write_png(Path(folder) / "b.png", IMAGE[:2], 0)
            with self.assertRaises(ValueError):
                convert([Path(folder) / "a.png", Path(folder) / "b.png"], "cat")


if __name__ == "__main__":
    unittest.main()
