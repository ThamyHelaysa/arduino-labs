// weather.h: busca o tempo da cidade configurada no Open-Meteo
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

const char* WEATHER_URL =
  "https://api.open-meteo.com/v1/forecast"
  "?latitude=" LATITUDE "&longitude=" LONGITUDE
  "&current=temperature_2m,weather_code"
  "&hourly=precipitation_probability,weather_code"
  "&daily=temperature_2m_max"
  "&timezone=America%2FSao_Paulo"
  "&past_days=1"
  "&forecast_days=1";

const int HEAT_THRESHOLD = 36;  // °C de máxima: a partir daqui, Recado de calor
const int RAIN_THRESHOLD = 60;  // %: a partir daqui, "vai chover"
const int COMPARISON_MARGIN = 2;  // °C: abaixo disso, "igual a ontem"

// Dados da última busca que deu certo
bool weatherOk     = false;  // já teve alguma busca com sucesso?
int  temperature = 0;
int  currentCode = 0;
int  rainChance = 0;      // maior chance de agora até 23h
bool rainExpected   = false;
bool storm  = false;
String todayDate  = "";     // ex.: 2026-11-07 (vem da resposta do Open-Meteo)
bool hasMaxToday  = false;  // a API mandou a máxima de hoje?
bool hasMaxYesterday = false;  // a API mandou a máxima de ontem?
int  maxToday     = 0;
int  maxYesterday    = 0;

bool isHotToday() {
  return weatherOk && hasMaxToday && maxToday >= HEAT_THRESHOLD;
}

// Recado do tempo mais importante, ou nullptr se não houver nenhum
const char* weatherNotice() {
  if (!weatherOk) return nullptr;
  if (storm)  return "TEMPESTADE HOJE. MELHOR NEM SAIR";
  if (isHotToday()) return "CALOR EXTREMO. BEBE ÁGUA";
  if (rainExpected)   return "VAI CHOVER. LEVA GUARDA-CHUVA";
  return nullptr;
}

bool isNoticeRed() {
  return weatherOk && (storm || isHotToday());
}

bool hasComparison() {
  return weatherOk && hasMaxToday && hasMaxYesterday;
}

// Só vale quando hasComparison() for verdadeiro
const char* comparisonVerdict() {
  int difference = maxToday - maxYesterday;
  if (difference <= -COMPARISON_MARGIN) return "MAIS FRIO";
  if (difference >= COMPARISON_MARGIN)  return "MAIS QUENTE";
  return "IGUAL A ONTEM";
}

bool connectWifi() {
  Serial.print("Conectando no Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void disconnectWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// Busca o tempo. Só troca os dados se TUDO der certo.
bool fetchWeather() {
  if (!connectWifi()) {
    Serial.println("Wi-Fi não conectou.");
    disconnectWifi();
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, WEATHER_URL);
  int status = http.GET();
  String response = (status == 200) ? http.getString() : "";
  http.end();
  disconnectWifi();

  if (status != 200) {
    Serial.printf("Erro ao buscar: %d\n", status);
    return false;
  }

  JsonDocument doc;
  if (deserializeJson(doc, response)) {
    Serial.println("Erro ao ler o JSON.");
    return false;
  }

  String now  = doc["current"]["time"].as<String>();
  String today   = now.substring(0, 10);
  int currentHour = now.substring(11, 13).toInt();

  int highestChance = 0;
  bool stormFound = false;
  JsonArray hours   = doc["hourly"]["time"];
  JsonArray chances = doc["hourly"]["precipitation_probability"];
  JsonArray codes = doc["hourly"]["weather_code"];
  for (int i = 0; i < hours.size(); i++) {
    String timestamp = hours[i].as<String>();
    if (!timestamp.startsWith(today)) continue;   // a resposta também traz ontem
    int hour = timestamp.substring(11, 13).toInt();
    if (hour < currentHour) continue;   // hora que já passou
    int chance = chances[i] | 0;
    if (chance > highestChance) highestChance = chance;
    if ((codes[i] | 0) >= 95) stormFound = true;
  }

  // Máximas: ontem é o dia logo antes de hoje na lista
  bool foundToday = false, foundYesterday = false;
  float highToday = 0, highYesterday = 0;
  JsonArray days    = doc["daily"]["time"];
  JsonArray highs = doc["daily"]["temperature_2m_max"];
  for (int i = 0; i < days.size(); i++) {
    if (days[i].as<String>() != today) continue;
    if (!highs[i].isNull()) {
      foundToday = true;
      highToday = highs[i].as<float>();
    }
    if (i > 0 && !highs[i - 1].isNull()) {
      foundYesterday = true;
      highYesterday = highs[i - 1].as<float>();
    }
    break;
  }

  // Deu tudo certo: agora sim troca os dados
  temperature = (int)round(doc["current"]["temperature_2m"].as<float>());
  currentCode = doc["current"]["weather_code"] | 0;
  rainChance = highestChance;
  rainExpected   = highestChance >= RAIN_THRESHOLD;
  storm  = stormFound;
  weatherOk     = true;
  todayDate    = today;
  hasMaxToday  = foundToday;
  hasMaxYesterday = foundYesterday;
  maxToday     = (int)round(highToday);
  maxYesterday    = (int)round(highYesterday);

  Serial.printf("Clima: %d °C, código %d, chuva %d%%, tempestade: %s, máx hoje %d, máx ontem %d\n",
    temperature, currentCode, rainChance, storm ? "sim" : "não", maxToday, maxYesterday);
  return true;
}