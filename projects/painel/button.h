// button.h: button on G32 (other side to GND)
const int BUTTON_PIN = 32;
const unsigned long LONG_PRESS_MS = 3000;  // 3 s
const unsigned long DEBOUNCE      = 50;

bool pressed = false;
bool longPressReported = false;
unsigned long pressStart = 0;

// 0 = nothing, 1 = short press, 2 = long press
int readButton() {
  bool isPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (isPressed && !pressed) {  // just pressed
    pressed = true;
    longPressReported = false;
    pressStart = millis();
  }
  if (isPressed && !longPressReported && millis() - pressStart >= LONG_PRESS_MS) {
    longPressReported = true;
    return 2;  // held for 3 s
  }
  if (!isPressed && pressed) {  // just released
    pressed = false;
    if (!longPressReported && millis() - pressStart >= DEBOUNCE) return 1;
  }
  return 0;
}
