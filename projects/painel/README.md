# Painel

Desk panel with a 2.9" e-ink display that shows the weather, a quote of the day and command cheat sheets. Weather data comes from [Open-Meteo](https://open-meteo.com/), with no API key.

## Parts

- ESP32
- 2.9" 3-color e-ink display, 296x128 (`GxEPD2_290_C90c` driver)
- One button

## Wiring

| Display | GPIO |
| --- | --- |
| CS | 5 |
| DC | 22 |
| RST | 21 |
| BUSY | 4 |
| SCK | 18 |
| SDI/MOSI | 23 |

The button connects GPIO 32 to GND.

## Setup

1. Copy `secrets.example.h` to `secrets.h`.
2. Fill in Wi-Fi, latitude and longitude. The timezone lives in `weather.h`.
3. Flash the board, replacing the port:

```sh
arduino-cli compile --profile esp32 --upload -p /dev/cu.usbserial-XXXX
```

## Usage

- Short press: cycles between the main screen and the cheat sheets.
- Long press (3 s) on the main screen: picks another quote, unless a weather notice is showing.

Quotes live in `quotes.h` and commands in `cheatsheets.h`.
