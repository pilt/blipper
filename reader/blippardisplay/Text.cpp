#include "Arduino.h"
#include "Text.h"
#include "Display.h"
#include "Font.h"

void textPrint(uint8_t c)
{
  uint8_t entry;
  if (c >= 0x20 && c < 0x80) {
    entry = c - 0x20;
  } else {
    entry = 0x5F;
  }
  for (int x = 0; x < FONT_NUM_COLS; x++)
  {
    displayWrite(font6x8[entry][x], DISPLAY_DATA);
  }
}

void textPrint(const uint8_t *text)
{
  const uint8_t *p = text;
  for (p = text; *p != 0; p++)
  {
    for (int x = 0; x < FONT_NUM_COLS; x++)
    {
      textPrint(*p);
    }
  }
}

