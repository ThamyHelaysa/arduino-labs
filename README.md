# arduino-labs

[![Compile](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml/badge.svg)](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml)

This is where I learn microcontrollers by building things I want on my desk. Each project is a self-contained Arduino sketch for the ESP32, and the tools are small scripts I wrote along the way to avoid flashing the board just to see if something fits on a screen.

## Projects

| Project | Description |
| --- | --- |
| [painel](projects/painel) | Desk panel with an e-ink display: weather, quote of the day and cheat sheets. |

<!-- photo: projects/painel/docs/images/hero.jpg (same hero photo as the painel README) -->

## Tools

Both tools came out of the painel, and both need only Python 3. Each folder has a README with every option.

### [fonts](tools/fonts)

I wrote this one when the fonts that ship with U8g2 stopped being enough and I wanted a different one on the panel. It has two scripts:

- `convert.py` turns a TTF, OTF or BDF font into a `.h` file the sketch can include. The painel's IBM BIOS fonts were made this way.
- `preview.py` draws a text in a given font to a PNG the size of the display and reports whether it fits. On a screen that takes about 15 seconds to refresh, this is much faster than flashing the board to find out a quote is one word too long.

```sh
python3 tools/fonts/convert.py VT323-Regular.ttf --name banner_font --size 16
python3 tools/fonts/preview.py "RAIN AHEAD. TAKE AN UMBRELLA" \
  --font-file banner_font.h --invert --center --max-lines 2 --size 296x40
```

### [bitmaps](tools/bitmaps)

This one converts PNGs into a `.h` file with bitmaps the display library can draw. I needed it for the weather icon on the panel's Main screen, which later became the Cat.

Each PNG becomes two arrays, one per ink: pure red pixels go to the red layer, dark pixels to the black layer, and everything else stays white. The sketch draws one array on top of the other.

```sh
python3 tools/bitmaps/convert.py art/*.png --name cat --scale 4 --output cat.h
```

## Building

Each project pins its board and libraries in `sketch.yaml`, so the [Arduino CLI](https://arduino.github.io/arduino-cli/) is all you need installed:

```sh
cd projects/painel
cp secrets.example.h secrets.h   # fill in your own values
arduino-cli compile --profile esp32
```

Wiring and flashing instructions are in each project's README.

## License

Code under [MIT](LICENSE). Third-party fonts keep their own license.
