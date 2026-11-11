#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "weather.h"
#include "quotes.h"
#include "button.h"
#include "cheatsheets.h"
#include "assets/fonts/IBM_BIOS/ibm_bios_font.h"
#include "assets/fonts/IBM_BIOS/ibm_bios_2y.h"

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(
  GxEPD2_290_C90c(5, 22, 21, 4)
);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// ===== Fonts =====
const uint8_t* const FONT_LARGE  = u8g2_font_VCR_OSD_tf;
const uint8_t* const FONT_BANNER = ibm_bios_2y;
const uint8_t* const FONT_TEXT   = ibm_bios_font;

// ===== Icons =====
const uint16_t ICON_CLOUD = 64;
const uint16_t ICON_RAIN  = 67;
const uint16_t ICON_SUN   = 69;

uint16_t weatherIcon(int code) {
  if (code >= 51) return ICON_RAIN;
  if (code >= 2)  return ICON_CLOUD;
  return ICON_SUN;
}

// ===== Quote of the day =====
int currentQuote = -1;
String quoteDay = "";

// Picks a quote different from the current one
void pickRandomQuote() {
  if (QUOTE_COUNT <= 1) { currentQuote = 0; return; }
  int candidate;
  do { candidate = random(QUOTE_COUNT); } while (candidate == currentQuote);
  currentQuote = candidate;
  Serial.printf("Quote: %s\n", QUOTES[currentQuote]);
}

// If the Open-Meteo date changed since the last quote, picks another one
void checkDayRollover() {
  if (todayDate.length() == 0) return;  // date still unknown
  if (quoteDay.length() == 0) {         // first run: only record the day
    quoteDay = todayDate;
    return;
  }
  if (todayDate != quoteDay) {
    Serial.println("New day: new quote.");
    pickRandomQuote();
    quoteDay = todayDate;
  }
}

// ===== Text helpers =====
// With draw = false, only computes where the text would end
int writeWrapped(const char* text, int x, int y, int width, int lineHeight, bool draw) {
  String t = String(text) + " ";
  String line = "";
  String currentWord = "";
  for (int i = 0; i < t.length(); i++) {
    char c = t[i];
    if (c == ' ') {
      String candidate = line.length() ? line + " " + currentWord : currentWord;
      if (u8g2Fonts.getUTF8Width(candidate.c_str()) > width && line.length()) {
        if (draw) {
          u8g2Fonts.setCursor(x, y);
          u8g2Fonts.print(line);
        }
        y += lineHeight;
        line = currentWord;
      } else {
        line = candidate;
      }
      currentWord = "";
    } else {
      currentWord += c;
    }
  }
  if (line.length()) {
    if (draw) {
      u8g2Fonts.setCursor(x, y);
      u8g2Fonts.print(line);
    }
    y += lineHeight;
  }
  return y;
}

void writeRightAligned(String text, int rightX, int y) {
  int width = u8g2Fonts.getUTF8Width(text.c_str());
  u8g2Fonts.setCursor(rightX - width, y);
  u8g2Fonts.print(text);
}

// ===== Banner =====
const int BANNER_HEIGHT      = 46;
const int BANNER_LINE_HEIGHT = 18;
const int BANNER_ASCENT      = 14;

// Draws the Banner with the text vertically centered and returns how many lines it used
int drawBanner(const char* text, uint16_t background) {
  display.fillRect(0, 0, 296, BANNER_HEIGHT, background);
  u8g2Fonts.setFont(FONT_BANNER);
  u8g2Fonts.setForegroundColor(GxEPD_WHITE);
  u8g2Fonts.setBackgroundColor(background);

  String upper = text;
  upper.toUpperCase();  // ASCII only: accented letters keep their case
  int lines = writeWrapped(upper.c_str(), 6, 0, 284, BANNER_LINE_HEIGHT, false) / BANNER_LINE_HEIGHT;
  int textHeight = BANNER_ASCENT + (lines - 1) * BANNER_LINE_HEIGHT;
  int y = (BANNER_HEIGHT - textHeight) / 2 + BANNER_ASCENT;
  writeWrapped(upper.c_str(), 6, y, 284, BANNER_LINE_HEIGHT, true);

  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  return lines;
}

String signedDegrees(int value) {
  return (value > 0 ? "+" : "") + String(value) + "°C";
}

// ===== Main screen =====
void drawMainScreen() {
  const char* notice = weatherNotice();
  const char* bannerText = notice ? notice : QUOTES[currentQuote];
  uint16_t bannerBackground = isNoticeRed() ? GxEPD_RED : GxEPD_BLACK;
  int bannerLines = 0;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    bannerLines = drawBanner(bannerText, bannerBackground);

    // --- Weather box ---
    display.drawRect(2, 50, 218, 76, GxEPD_BLACK);

    u8g2Fonts.setFont(FONT_LARGE);
    u8g2Fonts.setCursor(10, 74);
    u8g2Fonts.print(weatherOk ? String(temperature) + "°C" : String("--°C"));

    if (hasComparison()) {
      u8g2Fonts.setFont(FONT_BANNER);
      writeRightAligned(comparisonVerdict(), 214, 74);
    }

    u8g2Fonts.setFont(FONT_TEXT);
    if (!weatherOk) {
      u8g2Fonts.setCursor(10, 94);
      u8g2Fonts.print("No weather data");
    } else {
      if (hasComparison()) {
        u8g2Fonts.setCursor(10, 94);
        u8g2Fonts.print("Today " + signedDegrees(maxToday) + " : Yest. " + signedDegrees(maxYesterday));
      }
      u8g2Fonts.setCursor(10, 114);
      u8g2Fonts.print(String("Rain today ") + rainChance + "%");

      // --- Current weather icon ---
      u8g2Fonts.setFont(u8g2_font_open_iconic_weather_8x_t);
      u8g2Fonts.drawGlyph(226, 120, weatherIcon(currentCode));
    }
  } while (display.nextPage());

  if (bannerLines > 2) {
    Serial.println("WARNING: this text took more than 2 Banner lines. Consider shortening it.");
  }
}

// ===== Cheat sheets =====
int openScreen = 0;  // 0 = main, 1..N = cheat sheets

void drawCheatSheet(int sheetIndex) {
  const CheatSheet& cs = CHEATSHEETS[sheetIndex];

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Title bar: black background, white text
    display.fillRect(0, 0, 296, 18, GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setBackgroundColor(GxEPD_BLACK);  // glyph background = black, same as the bar
    u8g2Fonts.setCursor(6, 14);
    u8g2Fonts.print(cs.title);
    writeRightAligned(String(sheetIndex + 1) + "/" + String(CHEATSHEET_COUNT), 290, 14);

    // Commands
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // back to normal for the rest of the screen
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    for (int i = 0; i < 7 && cs.lines[i] != nullptr; i++) {
      u8g2Fonts.setCursor(6, 34 + i * 14);
      u8g2Fonts.print(cs.lines[i]);
    }
  } while (display.nextPage());
}

// Draws whichever screen is open
void drawScreen() {
  if (openScreen == 0) drawMainScreen();
  else drawCheatSheet(openScreen - 1);
}

// ===== Automatic refresh =====
const unsigned long WEATHER_INTERVAL = 30UL * 60 * 1000;      // 30 min
const unsigned long WEATHER_MAX_AGE  = 2UL * 60 * 60 * 1000;  // 2 h

unsigned long lastFetch   = 0;
unsigned long lastSuccess = 0;
String renderedScreen = "";

// Everything shown on screen, in a single line
String screenSummary() {
  if (openScreen > 0) return String("cheat|") + openScreen;
  const char* notice = weatherNotice();
  String r = (notice ? String(notice) : String(currentQuote)) + "|";
  if (!weatherOk) return r + "no data";
  r += String(temperature) + "|" + String(weatherIcon(currentCode)) + "|" + String(rainChance) + "|";
  if (hasComparison()) r += String(maxToday) + "/" + String(maxYesterday);
  return r;
}

void updateWeather() {
  lastFetch = millis();
  if (fetchWeather()) {
    lastSuccess = millis();
  } else if (weatherOk && millis() - lastSuccess >= WEATHER_MAX_AGE) {
    weatherOk = false;
    Serial.println("No update for over 2 h: showing 'No weather data'.");
  }
}

void redrawIfChanged() {
  String summary = screenSummary();
  if (summary == renderedScreen) {
    Serial.println("Nothing changed: screen kept, no flicker.");
    return;
  }
  Serial.println("Changed: redrawing the screen.");
  drawScreen();
  display.hibernate();
  renderedScreen = summary;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.init(115200);
  display.setRotation(1);
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

  updateWeather();
  pickRandomQuote();   // first quote on boot
  checkDayRollover();  // records today's date
  redrawIfChanged();
}

void loop() {
  int press = readButton();
  if (press == 1) {
    openScreen = (openScreen + 1) % (CHEATSHEET_COUNT + 1);
    Serial.printf("Short press: screen %d\n", openScreen);
    redrawIfChanged();
  } else if (press == 2) {
    if (openScreen != 0) {
      Serial.println("Long press ignored: only works on the main screen.");
    } else if (weatherNotice() != nullptr) {
      Serial.println("Long press ignored: Weather notice in the Banner.");
    } else {
      Serial.println("Long press: new quote.");
      pickRandomQuote();
      redrawIfChanged();
    }
  }

  if (millis() - lastFetch >= WEATHER_INTERVAL) {
    updateWeather();
    checkDayRollover();
    redrawIfChanged();
  }

  delay(10);  // shorter than before, so the button responds quickly
}
