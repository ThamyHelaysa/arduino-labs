# Bitmap tools

Turn PNGs into a `.h` with Adafruit GFX bitmaps for 3-color e-ink. Only needs Python 3.

## convert

```sh
python3 tools/bitmaps/convert.py art/*.png --name cat --output cat.h
```

Each PNG becomes two arrays, `<name>_<file>_black` and `<name>_<file>_red`, drawn with `display.drawBitmap`. All PNGs must have the same size, exported in `<NAME>_WIDTH` and `<NAME>_HEIGHT`.

Pure red (`#FF0000`, or close) goes to the red layer, dark pixels to black, and everything else, including transparency, stays white. Only 8-bit, non-interlaced RGB or RGBA PNGs are read; most editors export that by default.

Self-check:

```sh
python3 tools/bitmaps/test_convert.py
```
