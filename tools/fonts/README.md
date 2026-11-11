# Font tools

Test and convert U8g2 fonts without flashing the board. They only need Python 3.

## preview

Renders a display-sized PNG and reports whether the text fits. Exits with an error when a glyph or space is missing.

```sh
python3 tools/fonts/preview.py "RAIN AHEAD. TAKE AN UMBRELLA" \
  --font Pixellari_tf --invert --center --max-lines 2 --size 296x40
```

To see the installed fonts:

```sh
python3 tools/fonts/preview.py --list pixel
```

By default, fonts come from `~/Documents/Arduino/libraries/U8g2_for_Adafruit_GFX/src/u8g2_fonts.c`. Use `--library` for another path. Width is the sum of glyph advances, so it may differ by 1 px from the board. Only glyphs with codes up to 255 are read.

## convert

Turns a TTF, OTF or BDF into a `.h` for the sketch. The first time, build `bdfconv`:

```sh
tools/fonts/setup.sh
brew install otf2bdf   # only for TTF and OTF
```

```sh
python3 tools/fonts/convert.py VT323-Regular.ttf --name banner_font --size 16
python3 tools/fonts/preview.py "Accent test: ação, café" --font-file banner_font.h
```

For pixel fonts, `--size` must be the font's native size or a multiple of it, or the letters get distorted.

Before publishing a generated `.h`, check the font's license and keep the license file next to it.
