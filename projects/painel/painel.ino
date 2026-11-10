#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "weather.h"
#include "quotes.h"
#include "button.h"
#include "cheatsheets.h"

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(
  GxEPD2_290_C90c(5, 22, 21, 4)
);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// ===== Fontes =====
const uint8_t* const FONT_LARGE = u8g2_font_VCR_OSD_tf;
const uint8_t* const FONT_BANNER = u8g2_font_Pixellari_tf;
const uint8_t* const FONT_TEXT  = u8g2_font_nokiafc22_tf;

// ===== Ícones =====
const uint16_t ICON_CLOUD = 64;
const uint16_t ICON_RAIN = 67;
const uint16_t ICON_SUN   = 69;

uint16_t weatherIcon(int code) {
  if (code >= 51) return ICON_RAIN;
  if (code >= 2)  return ICON_CLOUD;
  return ICON_SUN;
}

// ===== Frase do dia =====
int currentQuote = -1;
String quoteDay = "";

// Sorteia uma frase diferente da atual
void pickRandomQuote() {
  if (QUOTE_COUNT <= 1) { currentQuote = 0; return; }
  int candidate;
  do { candidate = random(QUOTE_COUNT); } while (candidate == currentQuote);
  currentQuote = candidate;
  Serial.printf("Frase: %s\n", QUOTES[currentQuote]);
}

// Se a data do Open-Meteo mudou desde a última frase, sorteia outra
void checkDayRollover() {
  if (todayDate.length() == 0) return;              // ainda não sabe a data
  if (quoteDay.length() == 0) {                  // primeira vez: só anota o dia
    quoteDay = todayDate;
    return;
  }
  if (todayDate != quoteDay) {
    Serial.println("Virou o dia: frase nova.");
    pickRandomQuote();
    quoteDay = todayDate;
  }
}

// ===== Funções de texto =====
// Com desenhar = false, só calcula onde o texto terminaria
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
const int BANNER_HEIGHT       = 40;
const int BANNER_LINE_HEIGHT = 18;
const int BANNER_ASCENT   = 11;

// Desenha o Banner com o texto centralizado na altura e devolve quantas linhas usou
int drawBanner(const char* text, uint16_t background) {
  display.fillRect(0, 0, 296, BANNER_HEIGHT, background);
  u8g2Fonts.setFont(FONT_BANNER);
  u8g2Fonts.setForegroundColor(GxEPD_WHITE);
  u8g2Fonts.setBackgroundColor(background);

  int lines = writeWrapped(text, 6, 0, 284, BANNER_LINE_HEIGHT, false) / BANNER_LINE_HEIGHT;
  int textHeight = BANNER_ASCENT + (lines - 1) * BANNER_LINE_HEIGHT;
  int y = (BANNER_HEIGHT - textHeight) / 2 + BANNER_ASCENT;
  writeWrapped(text, 6, y, 284, BANNER_LINE_HEIGHT, true);

  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  return lines;
}

// ===== Tela principal =====
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

    // --- Caixa do tempo ---
    display.drawRect(2, 44, 218, 82, GxEPD_BLACK);

    u8g2Fonts.setFont(FONT_LARGE);
    u8g2Fonts.setCursor(10, 68);
    u8g2Fonts.print(weatherOk ? String(temperature) + "°C" : String("--°C"));

    if (hasComparison()) {
      u8g2Fonts.setFont(FONT_BANNER);
      writeRightAligned(comparisonVerdict(), 214, 68);
    }

    u8g2Fonts.setFont(FONT_TEXT);
    if (!weatherOk) {
      u8g2Fonts.setCursor(10, 92);
      u8g2Fonts.print("Sem dados do tempo");
    } else {
      if (hasComparison()) {
        u8g2Fonts.setCursor(10, 92);
        u8g2Fonts.print(String("Máx ") + maxToday + " hoje | " + maxYesterday + " ontem");
      }
      u8g2Fonts.setCursor(10, 114);
      u8g2Fonts.print(String("Chuva hoje ") + rainChance + "%");

      // --- Ícone do tempo agora ---
      u8g2Fonts.setFont(u8g2_font_open_iconic_weather_8x_t);
      u8g2Fonts.drawGlyph(226, 117, weatherIcon(currentCode));
    }
  } while (display.nextPage());

  if (bannerLines > 2) {
    Serial.println("AVISO: esse texto passou de 2 linhas no Banner. Considere encurtar.");
  }
}

// ===== Cheat sheets =====
int openScreen = 0;  // 0 = principal, 1..N = cheat sheets

void drawCheatSheet(int sheetIndex) {
  const CheatSheet& cs = CHEATSHEETS[sheetIndex];

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Barra do título: fundo preto, letra branca
    display.fillRect(0, 0, 296, 18, GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setBackgroundColor(GxEPD_BLACK);  // fundo das letras = preto, igual à barra
    u8g2Fonts.setCursor(6, 14);
    u8g2Fonts.print(cs.title);
    writeRightAligned(String(sheetIndex + 1) + "/" + String(CHEATSHEET_COUNT), 290, 14);

    // Comandos
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // volta ao normal para o resto da tela
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    for (int i = 0; i < 7 && cs.lines[i] != nullptr; i++) {
      u8g2Fonts.setCursor(6, 34 + i * 14);
      u8g2Fonts.print(cs.lines[i]);
    }
  } while (display.nextPage());
}

// Desenha a tela que estiver aberta
void drawScreen() {
  if (openScreen == 0) drawMainScreen();
  else drawCheatSheet(openScreen - 1);
}

// ===== Atualização automática =====
const unsigned long WEATHER_INTERVAL = 30UL * 60 * 1000;      // 30 min
const unsigned long WEATHER_MAX_AGE  = 2UL * 60 * 60 * 1000;  // 2 h

unsigned long lastFetch   = 0;
unsigned long lastSuccess = 0;
String renderedScreen = "";

// Tudo o que aparece na tela, numa linha só
String screenSummary() {
  if (openScreen > 0) return String("cheat|") + openScreen;
  const char* notice = weatherNotice();
  String r = (notice ? String(notice) : String(currentQuote)) + "|";
  if (!weatherOk) return r + "sem dados";
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
    Serial.println("Mais de 2 h sem atualizar: mostrando 'Sem dados'.");
  }
}

void redrawIfChanged() {
  String summary = screenSummary();
  if (summary == renderedScreen) {
    Serial.println("Nada mudou: tela mantida, sem piscar.");
    return;
  }
  Serial.println("Mudou: redesenhando a tela.");
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
  pickRandomQuote();          // primeira frase ao ligar
  checkDayRollover();   // anota o dia de hoje
  redrawIfChanged();
}

void loop() {
  int press = readButton();
  if (press == 1) {
    openScreen = (openScreen + 1) % (CHEATSHEET_COUNT + 1);
    Serial.printf("Aperto curto: tela %d\n", openScreen);
    redrawIfChanged();
  } else if (press == 2) {
    if (openScreen != 0) {
      Serial.println("Aperto longo ignorado: só funciona na tela principal.");
    } else if (weatherNotice() != nullptr) {
      Serial.println("Aperto longo ignorado: Recado do tempo no Banner.");
    } else {
      Serial.println("Aperto longo: frase nova.");
      pickRandomQuote();
      redrawIfChanged();
    }
  }

  if (millis() - lastFetch >= WEATHER_INTERVAL) {
    updateWeather();
    checkDayRollover();
    redrawIfChanged();
  }

  delay(10);  // mais curto que antes, para o botão responder rápido
}