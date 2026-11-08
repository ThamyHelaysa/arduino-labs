// botao.h: botão no G32 (o outro lado no GND)
const int PINO_BOTAO = 32;
const unsigned long TEMPO_LONGO = 3000;  // 3 s
const unsigned long DEBOUNCE    = 50;

bool apertado = false;
bool longoAvisado = false;
unsigned long inicioAperto = 0;

// 0 = nada, 1 = aperto curto, 2 = aperto longo
int lerBotao() {
  bool estaApertado = (digitalRead(PINO_BOTAO) == LOW);

  if (estaApertado && !apertado) {          // acabou de apertar
    apertado = true;
    longoAvisado = false;
    inicioAperto = millis();
  }
  if (estaApertado && !longoAvisado && millis() - inicioAperto >= TEMPO_LONGO) {
    longoAvisado = true;
    return 2;                                // completou 3 s segurando
  }
  if (!estaApertado && apertado) {          // acabou de soltar
    apertado = false;
    if (!longoAvisado && millis() - inicioAperto >= DEBOUNCE) return 1;
  }
  return 0;
}