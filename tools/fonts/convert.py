#!/usr/bin/env python3
"""Converts a TTF, OTF or BDF font into a .h file in U8g2 format."""

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from preview import load_font

BUILT_BDFCONV = Path(__file__).parent / ".build" / "bdfconv"
PORTUGUESE = "áàâãéêíóôõúçÁÀÂÃÉÊÍÓÔÕÚÇ°"


def find_bdfconv(option):
    found = option or shutil.which("bdfconv") or (BUILT_BDFCONV if BUILT_BDFCONV.exists() else None)
    if not found:
        raise SystemExit("bdfconv not found. Run tools/fonts/setup.sh.")
    return str(found)


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, errors="replace")
    if result.returncode != 0:
        details = (result.stderr or result.stdout).strip()
        raise SystemExit(f"Failed: {command[0]}\n{details}")


def to_bdf(source, size, folder):
    if source.suffix.lower() == ".bdf":
        return source
    if size is None:
        raise SystemExit("TTF or OTF fonts need --size, at the native size or a multiple of it.")
    otf2bdf = shutil.which("otf2bdf")
    if not otf2bdf:
        raise SystemExit("otf2bdf not found. On macOS: brew install otf2bdf.")
    target = Path(folder) / f"{source.stem}.bdf"
    run([otf2bdf, "-r", "72", "-p", str(size), "-o", str(target), str(source)])
    return target


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help=".ttf, .otf or .bdf file")
    parser.add_argument("--name", required=True, help="font name in the code")
    parser.add_argument("--size", type=int, help="size in pixels, required for TTF and OTF")
    parser.add_argument("--map", default="32-255", help="glyphs to include, default: 32-255")
    parser.add_argument("--output", type=Path, help="default: <name>.h")
    parser.add_argument("--bdfconv", help="path to bdfconv")
    return parser.parse_args()


def main():
    args = parse_args()
    if not re.fullmatch(r"[A-Za-z_]\w*", args.name):
        raise SystemExit("--name must be a valid C identifier.")
    if not args.source.is_file():
        raise SystemExit(f"File not found: {args.source}")
    output = args.output or Path(f"{args.name}.h")
    bdfconv = find_bdfconv(args.bdfconv)

    with tempfile.TemporaryDirectory() as folder:
        bdf = to_bdf(args.source, args.size, folder)
        run([bdfconv, "-f", "1", "-m", args.map, "-n", args.name, "-o", str(output), str(bdf)])

    font = load_font(output)
    missing = [c for c in PORTUGUESE if not font.has(c)]
    print(f"Generated: {output} ({len(font.offsets)} glyphs, height {font.max_height})")
    if missing:
        print("Warning, missing glyphs: " + " ".join(missing))
    print("Include it in the sketch after the U8g2 library:")
    print(f'  #include "{output.name}"')
    print(f"  u8g2Fonts.setFont({args.name});")
    return 0


if __name__ == "__main__":
    sys.exit(main())
