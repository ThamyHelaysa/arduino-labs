# Painel

<!-- photo: docs/images/hero.jpg (the panel on the desk, Main screen showing) -->

Painel is a small desk panel built around an ESP32 and a 2.9" e-ink display. It sits next to my keyboard, plugged into USB, and tells me three things: what the weather is doing today, a short quote to start the day, and the commands I keep having to look up.

There was no problem to solve here. I had a 3-color display that could not be used for a clock, because it has no partial refresh and every update makes the whole screen flash for about 15 seconds. A panel that changes only a few times a day turned that limit into a non-issue, and gave me an excuse to learn buttons, Wi-Fi and HTTP on a microcontroller.

Weather data comes from [Open-Meteo](https://open-meteo.com/), with no API key.

## What it shows

### Main screen

The Main screen has three parts:

- **Banner.** The inverted strip at the top. Most of the time it holds the Quote of the day. When the weather needs attention, a Weather notice takes its place: storm, heat or rain, in that order of priority. Storm and heat turn the Banner red, the only time red is used for text.
- **Weather box.** The current temperature, a verdict on whether today is colder, warmer or like yesterday, both days' highs, and the highest chance of rain from now until the end of the day.
- **Cat.** A pixel-art cat that changes Pose with the weather: sun, cloud, rain, heat, storm, or a confused one when there is no data.

<!-- photo: docs/images/main-screen.jpg (close-up of the Main screen with a Weather notice) -->

### Cheat sheets

Each Cheat sheet is one page with a title bar and up to seven commands. The ones in the repo are examples for AWS and Docker; the idea is to fill them with whatever I catch myself searching for.

<!-- photo: docs/images/cheat-sheet.jpg (a Cheat sheet page) -->

## How it behaves

The rules below are the ones that took the most thought, and they all follow from the slow, flashing refresh.

- **The screen only redraws when something visible changed.** Weather is fetched every 30 minutes, but if the temperature, the rain chance, the Pose and the Banner are all the same, the screen stays still.
- **Wi-Fi is on only during a fetch.** It connects, downloads the forecast and turns off again.
- **A failed fetch keeps the last data.** After 2 hours without a successful fetch, the panel stops pretending and shows `--°C` and "No weather data".
- **The quote changes once a day.** The panel has no clock, so it watches the date that comes back from Open-Meteo: the first successful fetch after midnight picks a new quote. A reboot picks a new one too.
- **Cheat sheets hide the weather, not stop it.** While a Cheat sheet is open, the weather keeps updating in the background without redrawing, and shows up when you return to the Main screen.

The thresholds live at the top of `weather.h`:

| Rule | Value |
| --- | --- |
| Rain notice | 60% chance or more in any remaining hour of the day |
| Heat notice | today's high at 36 °C or more |
| Storm notice | a thunderstorm weather code in any remaining hour of the day |
| "Like yesterday" | highs less than 2 °C apart |

## Parts

- ESP32 dev board
- 2.9" 3-color e-ink display (black, white and red), 296x128, `GxEPD2_290_C90c` driver
- One push button
- Breadboard and jumper wires

## Wiring

<!-- photo: docs/images/wiring.jpg (the breadboard with the display and the button wired) -->

| Display | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA/DIN (MOSI) | GPIO 23 |
| SCL/CLK (SCK) | GPIO 18 |
| CS | GPIO 5 |
| D/C | GPIO 22 |
| RES/RST | GPIO 21 |
| BUSY | GPIO 4 |

The button connects GPIO 32 to GND, using the pin's internal pull-up. GPIO 34 and 35 would not work here, since they are input-only and have no internal pull-up.

Unplug the USB cable before connecting or moving any wire.

## Setup

1. Copy `secrets.example.h` to `secrets.h`.
2. Fill in your Wi-Fi name and password, and your latitude and longitude. The timezone is set in the URL at the top of `weather.h`.
3. Flash the board, replacing the port:

```sh
arduino-cli compile --profile esp32 --upload -p /dev/cu.usbserial-XXXX
```

The board and library versions are pinned in `sketch.yaml`, so the build needs nothing else installed by hand.

## Usage

- **Short press:** goes to the next screen, from the Main screen through each Cheat sheet and back.
- **Long press (3 s) on the Main screen:** picks another quote. It is ignored while a Weather notice is in the Banner.

The button is not read while the screen is redrawing, so wait for the flashing to stop before pressing again.

Quotes live in `quotes.h` and commands in `cheatsheets.h`. About 60 characters fit in the Banner; [`tools/fonts`](../../tools/fonts) can check a text against the display size without flashing the board.

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

## How it got here

The panel grew one small step at a time, each one tested in its own sketch before joining the main one:

1. The display wired up and showing a hello world.
2. The button, telling a short press from a long one.
3. A font with accented characters.
4. The Main screen layout with fake data, then with the real forecast.
5. The Quote of the day and the Cheat sheets. This was the first version: a quote, a weather icon, the temperature and a line about rain.
6. A second look, inspired by a 1-bit panel I saw: inverted blocks, a pixel font, Weather notices with some personality, the comparison with yesterday, and the Cat in place of the weather icon.

The words in capitals (Banner, Weather notice, Pose and so on) are defined in the [glossary](GLOSSARY.md), and the decisions worth remembering are in [`docs/adr`](docs/adr).
