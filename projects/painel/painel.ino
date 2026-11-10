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
const uint8_t* const FONTE_GRANDE = u8g2_font_VCR_OSD_tf;
const uint8_t* const FONTE_BANNER = u8g2_font_Pixellari_tf;
const uint8_t* const FONTE_TEXTO  = u8g2_font_nokiafc22_tf;

// ===== Ícones =====
const uint16_t ICONE_NUVEM = 64;
const uint16_t ICONE_CHUVA = 67;
const uint16_t ICONE_SOL   = 69;

uint16_t iconeDoTempo(int codigo) {
  if (codigo >= 51) return ICONE_CHUVA;
  if (codigo >= 2)  return ICONE_NUVEM;
  return ICONE_SOL;
}

// ===== Frase do dia =====
int fraseAtual = -1;
String diaDaFrase = "";

// Sorteia uma frase diferente da atual
void sortearFrase() {
  if (TOTAL_FRASES <= 1) { fraseAtual = 0; return; }
  int nova;
  do { nova = random(TOTAL_FRASES); } while (nova == fraseAtual);
  fraseAtual = nova;
  Serial.printf("Frase: %s\n", FRASES[fraseAtual]);
}

// Se a data do Open-Meteo mudou desde a última frase, sorteia outra
void conferirViradaDoDia() {
  if (dataHoje.length() == 0) return;              // ainda não sabe a data
  if (diaDaFrase.length() == 0) {                  // primeira vez: só anota o dia
    diaDaFrase = dataHoje;
    return;
  }
  if (dataHoje != diaDaFrase) {
    Serial.println("Virou o dia: frase nova.");
    sortearFrase();
    diaDaFrase = dataHoje;
  }
}

// ===== Funções de texto =====
// Com desenhar = false, só calcula onde o texto terminaria
int escreverQuebrando(const char* texto, int x, int y, int largura, int alturaLinha, bool desenhar) {
  String t = String(texto) + " ";
  String linha = "";
  String palavra = "";
  for (int i = 0; i < t.length(); i++) {
    char c = t[i];
    if (c == ' ') {
      String teste = linha.length() ? linha + " " + palavra : palavra;
      if (u8g2Fonts.getUTF8Width(teste.c_str()) > largura && linha.length()) {
        if (desenhar) {
          u8g2Fonts.setCursor(x, y);
          u8g2Fonts.print(linha);
        }
        y += alturaLinha;
        linha = palavra;
      } else {
        linha = teste;
      }
      palavra = "";
    } else {
      palavra += c;
    }
  }
  if (linha.length()) {
    if (desenhar) {
      u8g2Fonts.setCursor(x, y);
      u8g2Fonts.print(linha);
    }
    y += alturaLinha;
  }
  return y;
}

void escreverDireita(String texto, int xDireita, int y) {
  int largura = u8g2Fonts.getUTF8Width(texto.c_str());
  u8g2Fonts.setCursor(xDireita - largura, y);
  u8g2Fonts.print(texto);
}

// ===== Banner =====
const int ALTURA_BANNER       = 40;
const int ALTURA_LINHA_BANNER = 18;
const int ASCENDENTE_BANNER   = 11;

// Desenha o Banner com o texto centralizado na altura e devolve quantas linhas usou
int desenharBanner(const char* texto, uint16_t fundo) {
  display.fillRect(0, 0, 296, ALTURA_BANNER, fundo);
  u8g2Fonts.setFont(FONTE_BANNER);
  u8g2Fonts.setForegroundColor(GxEPD_WHITE);
  u8g2Fonts.setBackgroundColor(fundo);

  int linhas = escreverQuebrando(texto, 6, 0, 284, ALTURA_LINHA_BANNER, false) / ALTURA_LINHA_BANNER;
  int alturaTexto = ASCENDENTE_BANNER + (linhas - 1) * ALTURA_LINHA_BANNER;
  int y = (ALTURA_BANNER - alturaTexto) / 2 + ASCENDENTE_BANNER;
  escreverQuebrando(texto, 6, y, 284, ALTURA_LINHA_BANNER, true);

  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  return linhas;
}

// ===== Tela principal =====
void desenharTelaPrincipal() {
  const char* recado = recadoDoTempo();
  const char* textoBanner = recado ? recado : FRASES[fraseAtual];
  uint16_t fundoBanner = recadoVermelho() ? GxEPD_RED : GxEPD_BLACK;
  int linhasBanner = 0;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    linhasBanner = desenharBanner(textoBanner, fundoBanner);

    // --- Caixa do tempo ---
    display.drawRect(2, 44, 218, 82, GxEPD_BLACK);

    u8g2Fonts.setFont(FONTE_GRANDE);
    u8g2Fonts.setCursor(10, 68);
    u8g2Fonts.print(climaOk ? String(temperatura) + "°C" : String("--°C"));

    if (temComparacao()) {
      u8g2Fonts.setFont(FONTE_BANNER);
      escreverDireita(vereditoComparacao(), 214, 68);
    }

    u8g2Fonts.setFont(FONTE_TEXTO);
    if (!climaOk) {
      u8g2Fonts.setCursor(10, 92);
      u8g2Fonts.print("Sem dados do tempo");
    } else {
      if (temComparacao()) {
        u8g2Fonts.setCursor(10, 92);
        u8g2Fonts.print(String("Máx ") + maxHoje + " hoje | " + maxOntem + " ontem");
      }
      u8g2Fonts.setCursor(10, 114);
      u8g2Fonts.print(String("Chuva hoje ") + chanceChuva + "%");

      // --- Ícone do tempo agora ---
      u8g2Fonts.setFont(u8g2_font_open_iconic_weather_8x_t);
      u8g2Fonts.drawGlyph(226, 117, iconeDoTempo(codigoAgora));
    }
  } while (display.nextPage());

  if (linhasBanner > 2) {
    Serial.println("AVISO: esse texto passou de 2 linhas no Banner. Considere encurtar.");
  }
}

// ===== Cheat sheets =====
int telaAberta = 0;  // 0 = principal, 1..N = cheat sheets

void desenharCheatSheet(int indice) {
  const CheatSheet& cs = CHEATSHEETS[indice];

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Barra do título: fundo preto, letra branca
    display.fillRect(0, 0, 296, 18, GxEPD_BLACK);
    u8g2Fonts.setFont(FONTE_TEXTO);
    u8g2Fonts.setForegroundColor(GxEPD_WHITE);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setBackgroundColor(GxEPD_BLACK);  // fundo das letras = preto, igual à barra
    u8g2Fonts.setCursor(6, 14);
    u8g2Fonts.print(cs.titulo);
    escreverDireita(String(indice + 1) + "/" + String(TOTAL_CHEATSHEETS), 290, 14);

    // Comandos
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // volta ao normal para o resto da tela
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    for (int i = 0; i < 7 && cs.linhas[i] != nullptr; i++) {
      u8g2Fonts.setCursor(6, 34 + i * 14);
      u8g2Fonts.print(cs.linhas[i]);
    }
  } while (display.nextPage());
}

// Desenha a tela que estiver aberta
void desenharTela() {
  if (telaAberta == 0) desenharTelaPrincipal();
  else desenharCheatSheet(telaAberta - 1);
}

// ===== Atualização automática =====
const unsigned long INTERVALO_CLIMA = 30UL * 60 * 1000;      // 30 min
const unsigned long VALIDADE_CLIMA  = 2UL * 60 * 60 * 1000;  // 2 h

unsigned long ultimaBusca   = 0;
unsigned long ultimoSucesso = 0;
String telaAtual = "";

// Tudo o que aparece na tela, numa linha só
String resumoDaTela() {
  if (telaAberta > 0) return String("cheat|") + telaAberta;
  const char* recado = recadoDoTempo();
  String r = (recado ? String(recado) : String(fraseAtual)) + "|";
  if (!climaOk) return r + "sem dados";
  r += String(temperatura) + "|" + String(iconeDoTempo(codigoAgora)) + "|" + String(chanceChuva) + "|";
  if (temComparacao()) r += String(maxHoje) + "/" + String(maxOntem);
  return r;
}

void atualizarClima() {
  ultimaBusca = millis();
  if (buscarClima()) {
    ultimoSucesso = millis();
  } else if (climaOk && millis() - ultimoSucesso >= VALIDADE_CLIMA) {
    climaOk = false;
    Serial.println("Mais de 2 h sem atualizar: mostrando 'Sem dados'.");
  }
}

void redesenharSeMudou() {
  String resumo = resumoDaTela();
  if (resumo == telaAtual) {
    Serial.println("Nada mudou: tela mantida, sem piscar.");
    return;
  }
  Serial.println("Mudou: redesenhando a tela.");
  desenharTela();
  display.hibernate();
  telaAtual = resumo;
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_BOTAO, INPUT_PULLUP);

  display.init(115200);
  display.setRotation(1);
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

  atualizarClima();
  sortearFrase();          // primeira frase ao ligar
  conferirViradaDoDia();   // anota o dia de hoje
  redesenharSeMudou();
}

void loop() {
  int aperto = lerBotao();
  if (aperto == 1) {
    telaAberta = (telaAberta + 1) % (TOTAL_CHEATSHEETS + 1);
    Serial.printf("Aperto curto: tela %d\n", telaAberta);
    redesenharSeMudou();
  } else if (aperto == 2) {
    if (telaAberta != 0) {
      Serial.println("Aperto longo ignorado: só funciona na tela principal.");
    } else if (recadoDoTempo() != nullptr) {
      Serial.println("Aperto longo ignorado: Recado do tempo no Banner.");
    } else {
      Serial.println("Aperto longo: frase nova.");
      sortearFrase();
      redesenharSeMudou();
    }
  }

  if (millis() - ultimaBusca >= INTERVALO_CLIMA) {
    atualizarClima();
    conferirViradaDoDia();
    redesenharSeMudou();
  }

  delay(10);  // mais curto que antes, para o botão responder rápido
}