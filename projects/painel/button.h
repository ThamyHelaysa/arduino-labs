// button.h: botão no G32 (o outro lado no GND)
const int BUTTON_PIN = 32;
const unsigned long LONG_PRESS_MS = 3000;  // 3 s
const unsigned long DEBOUNCE    = 50;

bool pressed = false;
bool longPressReported = false;
unsigned long pressStart = 0;

// 0 = nada, 1 = aperto curto, 2 = aperto longo
int readButton() {
  bool isPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (isPressed && !pressed) {          // acabou de apertar
    pressed = true;
    longPressReported = false;
    pressStart = millis();
  }
  if (isPressed && !longPressReported && millis() - pressStart >= LONG_PRESS_MS) {
    longPressReported = true;
    return 2;                                // completou 3 s segurando
  }
  if (!isPressed && pressed) {          // acabou de soltar
    pressed = false;
    if (!longPressReported && millis() - pressStart >= DEBOUNCE) return 1;
  }
  return 0;
}