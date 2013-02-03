#include "Display.h"
#include "Text.h"

void setup()
{
  displaySetup();
  displayClear();
}

void loop()
{
  displayGoto(0, 0);
  textPrint((const uint8_t *) "Hello Baljan");
  displayGoto(0, 2);
  textPrint((const uint8_t *) "This is Admittansen");
  displayGoto(0, 4);
  textPrint((const uint8_t *) "We heard you need");
  displayGoto(0, 5);
  textPrint((const uint8_t *) "a new display");
  /*displayGoto(0, 3);
  textPrint((const uint8_t *) "0123456789");
  displayGoto(0, 4);
  textPrint((const uint8_t *) "@ABCDEFGHIJKLMNO");
  displayGoto(0, 5);
  textPrint((const uint8_t *) "PQRSTUVWXYZ");
  displayGoto(0, 6);
  textPrint((const uint8_t *) " abcdefghijklmno");
  displayGoto(0, 7);
  textPrint((const uint8_t *) "pqrstuvwxyz    \x7f");*/
  /*for (int row = 0; row < 8; row++)
  {
    displayGoto(0, row);
    for (int col = 0; col < 132; col++)
    {
      displaySquare();
    }
  }
  for (int row = 0; row < 8; row++)
  {
    displayGoto(0, row);
    for (int col = 0; col < 132; col++)
    {
      displayBlank();
    }
  }*/
}
