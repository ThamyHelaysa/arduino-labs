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

## Cat

The Cat's art comes from the [1-bit characters pack by piiixl](https://piiixl.itch.io/1-bit-characters). The license does not allow redistribution, so the art and the generated `cat.h` stay out of the repo (see `.gitignore`). Without `cat.h`, the sketch builds with the placeholder Poses in `cat.example.h`.

With the pack in `~/Downloads/piixl-Char_1-bit`, run this from `projects/painel` to copy the six sprites and generate `cat.h`:

```sh
PACK=~/Downloads/piixl-Char_1-bit/16x16px
mkdir -p assets/cat
for pose in cloud:614 heat:583 no_data:586 rain:592 storm:590 sun:582; do
  cp "$PACK/char_ ${pose#*:}.png" "assets/cat/${pose%%:*}.png"
done
python3 ../../tools/bitmaps/convert.py assets/cat/*.png --name cat --scale 4 --output cat.h
```
