#!/usr/bin/env python3
"""Desenha um texto com uma fonte U8g2 em um PNG do tamanho da tela e diz se cabe."""

import argparse
import re
import struct
import sys
import zlib
from pathlib import Path

DEFAULT_LIBRARY = (
    Path.home() / "Documents/Arduino/libraries/U8g2_for_Adafruit_GFX/src/u8g2_fonts.c"
)
ARRAY = re.compile(
    r'const\s+uint8_t\s+(\w+)\s*\[\d*\]\s*'
    r'(?:U8G2_FONT_SECTION\("[^"]*"\))?\s*=\s*'
    r'((?:\s*"(?:[^"\\]|\\.)*")+)\s*;'
)
CHUNK = re.compile(r'"((?:[^"\\]|\\.)*)"')
SIMPLE_ESCAPES = {"n": 10, "t": 9, "r": 13, "b": 8, "f": 12, "v": 11, "a": 7}


def decode_c_string(raw):
    data = bytearray()
    i = 0
    while i < len(raw):
        if raw[i] != "\\":
            data.append(ord(raw[i]))
            i += 1
            continue
        octal = re.match(r"[0-7]{1,3}", raw[i + 1 :])
        if octal:
            data.append(int(octal.group(), 8))
            i += 1 + len(octal.group())
        else:
            data.append(SIMPLE_ESCAPES.get(raw[i + 1], ord(raw[i + 1])))
            i += 2
    return data


def font_arrays(source):
    """Devolve {nome: bloco de strings C} de um arquivo .c ou .h."""
    return {m.group(1): m.group(2) for m in ARRAY.finditer(source)}


class BitReader:
    def __init__(self, data, offset):
        self.data = data
        self.pos = offset * 8

    def unsigned(self, count):
        value = 0
        for i in range(count):
            byte = self.data[self.pos // 8]
            value |= ((byte >> (self.pos % 8)) & 1) << i
            self.pos += 1
        return value

    def signed(self, count):
        return self.unsigned(count) - (1 << (count - 1))


class Font:
    """Fonte no formato U8g2. Lê só os glifos de código até 255."""

    HEADER_SIZE = 23

    def __init__(self, name, data):
        self.name = name
        self.data = data
        self.bits_0, self.bits_1 = data[2], data[3]
        self.bits_w, self.bits_h, self.bits_x, self.bits_y, self.bits_d = data[4:9]
        self.max_width, self.max_height = data[9], data[10]
        self.ascent = self._int8(data[13])
        self.descent = self._int8(data[14])
        self.offsets = {}
        pos = self.HEADER_SIZE
        while pos + 1 < len(data) and data[pos + 1] != 0:
            self.offsets[data[pos]] = pos + 2
            pos += data[pos + 1]

    @staticmethod
    def _int8(value):
        return value - 256 if value > 127 else value

    def has(self, char):
        return ord(char) in self.offsets

    def _metrics(self, reader):
        width = reader.unsigned(self.bits_w)
        height = reader.unsigned(self.bits_h)
        x = reader.signed(self.bits_x)
        y = reader.signed(self.bits_y)
        delta = reader.signed(self.bits_d)
        return width, height, x, y, delta

    def advance(self, char):
        if not self.has(char):
            return 0
        return self._metrics(BitReader(self.data, self.offsets[ord(char)]))[4]

    def text_width(self, text):
        return sum(self.advance(c) for c in text)

    def glyph(self, char):
        """Devolve (largura, altura, x, y, avanço, pixels ligados como (coluna, linha))."""
        reader = BitReader(self.data, self.offsets[ord(char)])
        width, height, x, y, delta = self._metrics(reader)
        pixels = []
        column = row = 0

        def emit(length, on):
            nonlocal column, row
            for _ in range(length):
                if on:
                    pixels.append((column, row))
                column += 1
                if column == width:
                    column = 0
                    row += 1

        while height > 0 and row < height:
            zeros = reader.unsigned(self.bits_0)
            ones = reader.unsigned(self.bits_1)
            while True:
                emit(zeros, False)
                emit(ones, True)
                if reader.unsigned(1) == 0:
                    break
        return width, height, x, y, delta, pixels


def load_font(path, name=None):
    """Carrega uma fonte. Sem nome, o arquivo precisa ter uma fonte só."""
    arrays = font_arrays(Path(path).read_text(encoding="latin-1"))
    if name is None:
        if len(arrays) != 1:
            raise SystemExit(f"{path}: esperava uma fonte, achei {len(arrays)}. Use --font.")
        name = next(iter(arrays))
    key = name if name in arrays else f"u8g2_font_{name}"
    if key not in arrays:
        raise SystemExit(f"Fonte '{name}' não encontrada em {path}. Veja --list.")
    data = bytearray()
    for raw in CHUNK.findall(arrays[key]):
        data += decode_c_string(raw)
    return Font(key, data)


def wrap(text, font, width):
    """Quebra por palavras, do mesmo jeito que o sketch."""
    lines = []
    current = ""
    for word in text.split():
        candidate = f"{current} {word}" if current else word
        if current and font.text_width(candidate) > width:
            lines.append(current)
            current = word
        else:
            current = candidate
    if current:
        lines.append(current)
    return lines


def draw(canvas, width, height, font, text, x, baseline):
    for char in text:
        if not font.has(char):
            continue
        _, glyph_h, glyph_x, glyph_y, delta, pixels = font.glyph(char)
        top = baseline - glyph_h - glyph_y
        for column, row in pixels:
            px, py = x + glyph_x + column, top + row
            if 0 <= px < width and 0 <= py < height:
                canvas[py * width + px] = 1
        x += delta


def write_png(path, canvas, width, height, scale, invert):
    ink, paper = (255, 0) if invert else (0, 255)
    rows = bytearray()
    for y in range(height):
        row = bytearray()
        for x in range(width):
            row += bytes([ink if canvas[y * width + x] else paper]) * scale
        rows += (b"\0" + bytes(row)) * scale

    def chunk(kind, body):
        crc = zlib.crc32(kind + body)
        return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", crc)

    header = struct.pack(">IIBBBBB", width * scale, height * scale, 8, 0, 0, 0, 0)
    Path(path).write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(bytes(rows), 9))
        + chunk(b"IEND", b"")
    )


def parse_size(value):
    match = re.fullmatch(r"(\d+)x(\d+)", value)
    if not match:
        raise argparse.ArgumentTypeError("use LARGURAxALTURA, por exemplo 296x128")
    return int(match.group(1)), int(match.group(2))


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("text", nargs="?", help="texto a desenhar")
    parser.add_argument("--font", help="nome da fonte, por exemplo Pixellari_tf")
    parser.add_argument("--font-file", help="arquivo .h gerado pelo convert.py")
    parser.add_argument("--library", default=DEFAULT_LIBRARY, help="caminho do u8g2_fonts.c")
    parser.add_argument("--list", nargs="?", const="", metavar="FILTRO", help="lista as fontes")
    parser.add_argument("--size", type=parse_size, default=(296, 128), help="padrão: 296x128")
    parser.add_argument("--margin", type=int, default=6, help="margem em pixels, padrão: 6")
    parser.add_argument("--line-height", type=int, help="padrão: altura da fonte")
    parser.add_argument("--max-lines", type=int, help="falha se passar deste número de linhas")
    parser.add_argument("--center", action="store_true", help="centraliza na altura")
    parser.add_argument("--invert", action="store_true", help="texto branco em fundo preto")
    parser.add_argument("--scale", type=int, default=3, help="zoom do PNG, padrão: 3")
    parser.add_argument("--output", default="preview.png", help="padrão: preview.png")
    return parser.parse_args()


def main():
    args = parse_args()

    if args.list is not None:
        names = font_arrays(Path(args.library).read_text(encoding="latin-1"))
        for name in sorted(names):
            short = name.removeprefix("u8g2_font_")
            if args.list.lower() in short.lower():
                print(short)
        return 0

    if not args.text:
        raise SystemExit("Informe o texto.")
    if args.font_file:
        font = load_font(args.font_file, args.font)
    elif args.font:
        font = load_font(args.library, args.font)
    else:
        raise SystemExit("Informe --font ou --font-file.")

    width, height = args.size
    available = width - 2 * args.margin
    line_height = args.line_height or font.max_height
    lines = wrap(args.text, font, available)
    block = font.ascent + (len(lines) - 1) * line_height
    first = (height - block) // 2 + font.ascent if args.center else args.margin + font.ascent

    canvas = bytearray(width * height)
    for i, line in enumerate(lines):
        draw(canvas, width, height, font, line, args.margin, first + i * line_height)
    write_png(args.output, canvas, width, height, args.scale, args.invert)

    missing = sorted({c for c in args.text if c != " " and not font.has(c)})
    last_bottom = first + (len(lines) - 1) * line_height - font.descent
    problems = []
    if missing:
        problems.append(f"sem glifo para: {' '.join(missing)}")
    if any(font.text_width(line) > available for line in lines):
        problems.append(f"linha mais larga que {available} px")
    if first - font.ascent < 0 or last_bottom > height:
        problems.append(f"texto passa da altura de {height} px")
    if args.max_lines and len(lines) > args.max_lines:
        problems.append(f"{len(lines)} linhas, limite {args.max_lines}")

    print(f"Fonte: {font.name} (altura {font.max_height}, ascendente {font.ascent})")
    for line in lines:
        print(f"  {font.text_width(line):4d} px  {line}")
    print(f"Linhas: {len(lines)}, altura de linha: {line_height}, primeira base: y={first}")
    print(f"PNG: {args.output}")
    if problems:
        print("Não cabe: " + "; ".join(problems))
        return 1
    print("Cabe.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
