// weather.h: busca o tempo da cidade configurada no Open-Meteo
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

const char* URL_CLIMA =
  "https://api.open-meteo.com/v1/forecast"
  "?latitude=" LATITUDE "&longitude=" LONGITUDE
  "&current=temperature_2m,weather_code"
  "&hourly=precipitation_probability,weather_code"
  "&daily=temperature_2m_max"
  "&timezone=America%2FSao_Paulo"
  "&past_days=1"
  "&forecast_days=1";

const int LIMITE_CALOR = 36;  // °C de máxima: a partir daqui, Recado de calor
const int LIMITE_CHUVA = 60;  // %: a partir daqui, "vai chover"
const int MARGEM_COMPARACAO = 2;  // °C: abaixo disso, "igual a ontem"

// Dados da última busca que deu certo
bool climaOk     = false;  // já teve alguma busca com sucesso?
int  temperatura = 0;
int  codigoAgora = 0;
int  chanceChuva = 0;      // maior chance de agora até 23h
bool vaiChover   = false;
bool tempestade  = false;
String dataHoje  = "";     // ex.: 2026-11-07 (vem da resposta do Open-Meteo)
bool temMaxHoje  = false;  // a API mandou a máxima de hoje?
bool temMaxOntem = false;  // a API mandou a máxima de ontem?
int  maxHoje     = 0;
int  maxOntem    = 0;

bool calorHoje() {
  return climaOk && temMaxHoje && maxHoje >= LIMITE_CALOR;
}

// Recado do tempo mais importante, ou nullptr se não houver nenhum
const char* recadoDoTempo() {
  if (!climaOk) return nullptr;
  if (tempestade)  return "TEMPESTADE HOJE. MELHOR NEM SAIR";
  if (calorHoje()) return "CALOR EXTREMO. BEBE ÁGUA";
  if (vaiChover)   return "VAI CHOVER. LEVA GUARDA-CHUVA";
  return nullptr;
}

bool recadoVermelho() {
  return climaOk && (tempestade || calorHoje());
}

bool temComparacao() {
  return climaOk && temMaxHoje && temMaxOntem;
}

// Só vale quando temComparacao() for verdadeiro
const char* vereditoComparacao() {
  int diferenca = maxHoje - maxOntem;
  if (diferenca <= -MARGEM_COMPARACAO) return "MAIS FRIO";
  if (diferenca >= MARGEM_COMPARACAO)  return "MAIS QUENTE";
  return "IGUAL A ONTEM";
}

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
  String hoje   = agora.substring(0, 10);
  int horaAgora = agora.substring(11, 13).toInt();

  int maiorChance = 0;
  bool temTempestade = false;
  JsonArray horas   = doc["hourly"]["time"];
  JsonArray chances = doc["hourly"]["precipitation_probability"];
  JsonArray codigos = doc["hourly"]["weather_code"];
  for (int i = 0; i < horas.size(); i++) {
    String horario = horas[i].as<String>();
    if (!horario.startsWith(hoje)) continue;   // a resposta também traz ontem
    int hora = horario.substring(11, 13).toInt();
    if (hora < horaAgora) continue;   // hora que já passou
    int chance = chances[i] | 0;
    if (chance > maiorChance) maiorChance = chance;
    if ((codigos[i] | 0) >= 95) temTempestade = true;
  }

  // Máximas: ontem é o dia logo antes de hoje na lista
  bool achouHoje = false, achouOntem = false;
  float maximaHoje = 0, maximaOntem = 0;
  JsonArray dias    = doc["daily"]["time"];
  JsonArray maximas = doc["daily"]["temperature_2m_max"];
  for (int i = 0; i < dias.size(); i++) {
    if (dias[i].as<String>() != hoje) continue;
    if (!maximas[i].isNull()) {
      achouHoje = true;
      maximaHoje = maximas[i].as<float>();
    }
    if (i > 0 && !maximas[i - 1].isNull()) {
      achouOntem = true;
      maximaOntem = maximas[i - 1].as<float>();
    }
    break;
  }

  // Deu tudo certo: agora sim troca os dados
  temperatura = (int)round(doc["current"]["temperature_2m"].as<float>());
  codigoAgora = doc["current"]["weather_code"] | 0;
  chanceChuva = maiorChance;
  vaiChover   = maiorChance >= LIMITE_CHUVA;
  tempestade  = temTempestade;
  climaOk     = true;
  dataHoje    = hoje;
  temMaxHoje  = achouHoje;
  temMaxOntem = achouOntem;
  maxHoje     = (int)round(maximaHoje);
  maxOntem    = (int)round(maximaOntem);

  Serial.printf("Clima: %d °C, código %d, chuva %d%%, tempestade: %s, máx hoje %d, máx ontem %d\n",
    temperatura, codigoAgora, chanceChuva, tempestade ? "sim" : "não", maxHoje, maxOntem);
  return true;
}