// clima.h: busca o tempo da cidade configurada no Open-Meteo
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "segredos.h"

const char* URL_CLIMA =
  "https://api.open-meteo.com/v1/forecast"
  "?latitude=LATITUDE&longitude=LONGITUDE"
  "&current=temperature_2m,weather_code"
  "&hourly=precipitation_probability,weather_code"
  "&timezone=America%2FSao_Paulo"
  "&forecast_days=1";

const int LIMITE_CALOR = 36;  // °C: a partir daqui, alerta
const int LIMITE_CHUVA = 60;  // %: a partir daqui, "vai chover"

// Dados da última busca que deu certo
bool climaOk     = false;  // já teve alguma busca com sucesso?
int  temperatura = 0;
int  codigoAgora = 0;
int  chanceChuva = 0;      // maior chance de agora até 23h
bool vaiChover   = false;
bool tempestade  = false;
String dataHoje  = "";     // ex.: 2026-11-07 (vem da resposta do Open-Meteo)

bool conectarWifi() {
  Serial.print("Conectando no Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NOME, WIFI_SENHA);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void desligarWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// Busca o tempo. Só troca os dados se TUDO der certo.
bool buscarClima() {
  if (!conectarWifi()) {
    Serial.println("Wi-Fi não conectou.");
    desligarWifi();
    return false;
  }

  WiFiClientSecure cliente;
  cliente.setInsecure();
  HTTPClient http;
  http.begin(cliente, URL_CLIMA);
  int status = http.GET();
  String resposta = (status == 200) ? http.getString() : "";
  http.end();
  desligarWifi();

  if (status != 200) {
    Serial.printf("Erro ao buscar: %d\n", status);
    return false;
  }

  JsonDocument doc;
  if (deserializeJson(doc, resposta)) {
    Serial.println("Erro ao ler o JSON.");
    return false;
  }

  String agora  = doc["current"]["time"].as<String>();
  int horaAgora = agora.substring(11, 13).toInt();

  int maiorChance = 0;
  bool temTempestade = false;
  JsonArray horas   = doc["hourly"]["time"];
  JsonArray chances = doc["hourly"]["precipitation_probability"];
  JsonArray codigos = doc["hourly"]["weather_code"];
  for (int i = 0; i < horas.size(); i++) {
    int hora = String(horas[i].as<const char*>()).substring(11, 13).toInt();
    if (hora < horaAgora) continue;   // hora que já passou
    int chance = chances[i] | 0;
    if (chance > maiorChance) maiorChance = chance;
    if ((codigos[i] | 0) >= 95) temTempestade = true;
  }

  // Deu tudo certo: agora sim troca os dados
  temperatura = (int)round(doc["current"]["temperature_2m"].as<float>());
  codigoAgora = doc["current"]["weather_code"] | 0;
  chanceChuva = maiorChance;
  vaiChover   = maiorChance >= LIMITE_CHUVA;
  tempestade  = temTempestade;
  climaOk     = true;
  dataHoje    = agora.substring(0, 10);

  Serial.printf("Clima: %d °C, código %d, chuva %d%%, tempestade: %s\n",
    temperatura, codigoAgora, chanceChuva, tempestade ? "sim" : "não");
  return true;
}