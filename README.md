# arduino-labs

[![Compile](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml/badge.svg)](https://github.com/ThamyHelaysa/arduino-labs/actions/workflows/compile.yml)

Arduino and ESP32 projects and experiments.

## Projects

| Project | Description |
| --- | --- |
| [painel](projects/painel) | Desk panel with an e-ink display: weather, quote of the day and cheat sheets. |

## Tools

| Tool | Description |
| --- | --- |
| [fonts](tools/fonts) | Checks in a PNG whether text fits the display and converts fonts to U8g2. |

## Building

Each project pins its board and libraries in `sketch.yaml`. With the [Arduino CLI](https://arduino.github.io/arduino-cli/) installed:

```sh
cd projects/painel
cp secrets.example.h secrets.h   # fill in your own values
arduino-cli compile --profile esp32
```

## License

Code under [MIT](LICENSE). Third-party fonts keep their own license.
