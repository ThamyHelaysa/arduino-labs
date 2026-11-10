#!/usr/bin/env python3
"""Converte uma fonte TTF, OTF ou BDF em um arquivo .h no formato U8g2."""

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
        raise SystemExit("bdfconv não encontrado. Rode tools/fonts/setup.sh.")
    return str(found)


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, errors="replace")
    if result.returncode != 0:
        details = (result.stderr or result.stdout).strip()
        raise SystemExit(f"Falhou: {command[0]}\n{details}")


def to_bdf(source, size, folder):
    if source.suffix.lower() == ".bdf":
        return source
    if size is None:
        raise SystemExit("Fonte TTF ou OTF precisa de --size, no tamanho nativo ou múltiplo dele.")
    otf2bdf = shutil.which("otf2bdf")
    if not otf2bdf:
        raise SystemExit("otf2bdf não encontrado. No macOS: brew install otf2bdf.")
    target = Path(folder) / f"{source.stem}.bdf"
    run([otf2bdf, "-r", "72", "-p", str(size), "-o", str(target), str(source)])
    return target


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="arquivo .ttf, .otf ou .bdf")
    parser.add_argument("--name", required=True, help="nome da fonte no código")
    parser.add_argument("--size", type=int, help="tamanho em pixels, obrigatório para TTF e OTF")
    parser.add_argument("--map", default="32-255", help="glifos a incluir, padrão: 32-255")
    parser.add_argument("--output", type=Path, help="padrão: <nome>.h")
    parser.add_argument("--bdfconv", help="caminho do bdfconv")
    return parser.parse_args()


def main():
    args = parse_args()
    if not re.fullmatch(r"[A-Za-z_]\w*", args.name):
        raise SystemExit("--name precisa ser um identificador válido em C.")
    if not args.source.is_file():
        raise SystemExit(f"Arquivo não encontrado: {args.source}")
    output = args.output or Path(f"{args.name}.h")
    bdfconv = find_bdfconv(args.bdfconv)

    with tempfile.TemporaryDirectory() as folder:
        bdf = to_bdf(args.source, args.size, folder)
        run([bdfconv, "-f", "1", "-m", args.map, "-n", args.name, "-o", str(output), str(bdf)])

    font = load_font(output)
    missing = [c for c in PORTUGUESE if not font.has(c)]
    print(f"Gerado: {output} ({len(font.offsets)} glifos, altura {font.max_height})")
    if missing:
        print("Atenção, faltam glifos: " + " ".join(missing))
    print("Inclua no sketch depois da biblioteca U8g2:")
    print(f'  #include "{output.name}"')
    print(f"  u8g2Fonts.setFont({args.name});")
    return 0


if __name__ == "__main__":
    sys.exit(main())
