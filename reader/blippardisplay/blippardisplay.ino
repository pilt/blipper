#include "Display.h"
#include "Text.h"

void setup()
{
  displaySetup();
  displayClear();
}

void loop()
{
  for (int row = 0; row < 8; row++) {
    displayGoto(0, row);
    for (int col = 0; col < 16; col++) {
      textPrint((uint8_t) (16*(row+0) + col));
    }
  }
  delay(1000);
}
