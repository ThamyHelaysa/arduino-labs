// quotes.h: one quote per line, in quotes, with a trailing comma.
// About 60 characters fit in the 2 Banner lines. Longer text spills into the Weather box.
// Check the fit with tools/fonts/preview.py.
const char* QUOTES[] = {
  "Done is better than perfect. One step at a time.",
  "Slow and steady goes a long way.",
  "Constant dripping wears away the stone.",
  "Nothing ventured, nothing gained.",
  "Little by little, the bird builds its nest.",
  "Better late than never.",
  "Small steps, every day.",
  "Breathe. One thing at a time.",
};
const int QUOTE_COUNT = sizeof(QUOTES) / sizeof(QUOTES[0]);
