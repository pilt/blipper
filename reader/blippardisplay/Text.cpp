#include "Arduino.h"
#include "Text.h"
#include "Display.h"
#include "Font.h"

void textPrint(const uint8_t *text)
{
  const uint8_t *p = text;
  for (p = text; *p != 0; p++)
  {
    for (int x = 0; x < FONT_NUM_COLS; x++)
    {
      displayWrite(font6x8[(*p - 0x20)][x], DISPLAY_DATA);
    }
  }
}

