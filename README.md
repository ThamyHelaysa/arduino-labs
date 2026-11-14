# arduino-labs

[![Compile](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml/badge.svg)](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml)

This is where I learn microcontrollers by building things I want on my desk. Each project is a self-contained Arduino sketch for the ESP32, and the tools are small scripts I wrote along the way to avoid flashing the board just to see if something fits on a screen.

## Projects

| Project | Description |
| --- | --- |
| [painel](projects/painel) | Desk panel with an e-ink display: weather, quote of the day and cheat sheets. |

<!-- photo: projects/painel/docs/images/hero.jpg (same hero photo as the painel README) -->

## Tools

| Tool | Description |
| --- | --- |
| [fonts](tools/fonts) | Checks in a PNG whether text fits the display and converts fonts to U8g2. |
| [bitmaps](tools/bitmaps) | Turns PNGs into Adafruit GFX bitmaps for 3-color e-ink. |

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
