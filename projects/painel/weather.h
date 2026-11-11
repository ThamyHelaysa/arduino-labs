// weather.h: fetches the configured city's weather from Open-Meteo
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

const int HEAT_THRESHOLD    = 36;  // °C high: from here on, heat notice
const int RAIN_THRESHOLD    = 60;  // %: from here on, rain notice
const int COMPARISON_MARGIN = 2;   // °C: below this, "like yesterday"

// Data from the last successful fetch
bool   weatherOk       = false;  // has any fetch succeeded yet?
int    temperature     = 0;
int    currentCode     = 0;
int    rainChance      = 0;      // highest chance from now until 23h
bool   rainExpected    = false;
bool   storm           = false;
String todayDate       = "";     // e.g. 2026-11-07 (from the Open-Meteo response)
bool   hasMaxToday     = false;  // did the API send today's high?
bool   hasMaxYesterday = false;  // did the API send yesterday's high?
int    maxToday        = 0;
int    maxYesterday    = 0;

bool isHotToday() {
  return weatherOk && hasMaxToday && maxToday >= HEAT_THRESHOLD;
}

// Most important Weather notice, or nullptr when there is none
const char* weatherNotice() {
  if (!weatherOk) return nullptr;
  if (storm)        return "STORM TODAY. BETTER STAY INSIDE";
  if (isHotToday()) return "EXTREME HEAT. DRINK WATER";
  if (rainExpected) return "RAIN AHEAD. TAKE AN UMBRELLA";
  return nullptr;
}

bool isNoticeRed() {
  return weatherOk && (storm || isHotToday());
}

bool hasComparison() {
  return weatherOk && hasMaxToday && hasMaxYesterday;
}

// Only valid when hasComparison() is true
const char* comparisonVerdict() {
  int difference = maxToday - maxYesterday;
  if (difference <= -COMPARISON_MARGIN) return "COLDER";
  if (difference >= COMPARISON_MARGIN)  return "WARMER";
  return "LIKE YESTERDAY";
}

bool connectWifi() {
  Serial.print("Connecting to Wi-Fi");
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

// Fetches the weather. Replaces the data only if EVERYTHING succeeds.
bool fetchWeather() {
  if (!connectWifi()) {
    Serial.println("Wi-Fi did not connect.");
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
    Serial.printf("Fetch failed: %d\n", status);
    return false;
  }

  JsonDocument doc;
  if (deserializeJson(doc, response)) {
    Serial.println("Failed to parse the JSON.");
    return false;
  }

  String now      = doc["current"]["time"].as<String>();
  String today    = now.substring(0, 10);
  int currentHour = now.substring(11, 13).toInt();

  int highestChance = 0;
  bool stormFound = false;
  JsonArray hours   = doc["hourly"]["time"];
  JsonArray chances = doc["hourly"]["precipitation_probability"];
  JsonArray codes   = doc["hourly"]["weather_code"];
  for (int i = 0; i < hours.size(); i++) {
    String timestamp = hours[i].as<String>();
    if (!timestamp.startsWith(today)) continue;  // the response also includes yesterday
    int hour = timestamp.substring(11, 13).toInt();
    if (hour < currentHour) continue;  // hour already passed
    int chance = chances[i] | 0;
    if (chance > highestChance) highestChance = chance;
    if ((codes[i] | 0) >= 95) stormFound = true;
  }

  // Highs: yesterday is the day right before today in the list
  bool foundToday = false, foundYesterday = false;
  float highToday = 0, highYesterday = 0;
  JsonArray days  = doc["daily"]["time"];
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

  // Everything succeeded: now replace the data
  temperature     = (int)round(doc["current"]["temperature_2m"].as<float>());
  currentCode     = doc["current"]["weather_code"] | 0;
  rainChance      = highestChance;
  rainExpected    = highestChance >= RAIN_THRESHOLD;
  storm           = stormFound;
  weatherOk       = true;
  todayDate       = today;
  hasMaxToday     = foundToday;
  hasMaxYesterday = foundYesterday;
  maxToday        = (int)round(highToday);
  maxYesterday    = (int)round(highYesterday);

  Serial.printf("Weather: %d °C, code %d, rain %d%%, storm: %s, high today %d, high yesterday %d\n",
    temperature, currentCode, rainChance, storm ? "yes" : "no", maxToday, maxYesterday);
  return true;
}
