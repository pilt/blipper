#include "Arduino.h"
#include "Display.h"

const int shiftRegDataPin   =  2;
const int shiftRegClockPin  =  3;
const int dataCommandInvPin = 14;
const int writeInvPin       = 15;
//const int readInvPin        = 16;
const int slaveSelectInvPin = 17;
const int resetInvPin       = 16;

static void displaySetData(uint8_t x);

void displaySetup()
{
  pinMode(shiftRegDataPin, OUTPUT);
  digitalWrite(shiftRegDataPin, HIGH);
  pinMode(shiftRegClockPin, OUTPUT);
  digitalWrite(shiftRegClockPin, LOW);
  pinMode(dataCommandInvPin, OUTPUT);
  digitalWrite(dataCommandInvPin, HIGH);
  pinMode(writeInvPin, OUTPUT);
  digitalWrite(writeInvPin, HIGH);
  //pinMode(readInvPin, OUTPUT);
  //digitalWrite(readInvPin, HIGH);
  pinMode(slaveSelectInvPin, OUTPUT);
  digitalWrite(slaveSelectInvPin, HIGH);
  pinMode(resetInvPin, OUTPUT);
  digitalWrite(resetInvPin, HIGH);

  displayReset();
}

void displayReset()
{
  digitalWrite(resetInvPin, LOW);
  delay(2);
  digitalWrite(resetInvPin, HIGH);

  displayWrite(0xae, DISPLAY_COMMAND); /* set display off */
  displayWrite(0x02, DISPLAY_COMMAND); /* set lower column start address */
  displayWrite(0x10, DISPLAY_COMMAND); /* set higher column start address */
  displayWrite(0x40, DISPLAY_COMMAND); /* set display start line */
  displayWrite(0x2E, DISPLAY_COMMAND);
  displayWrite(0x81, DISPLAY_COMMAND); /* set contrast control */
  displayWrite(0x32, DISPLAY_COMMAND);
  displayWrite(0x82, DISPLAY_COMMAND);
  displayWrite(0x80, DISPLAY_COMMAND);
  displayWrite(0xa1, DISPLAY_COMMAND); /* set segment remap */
  displayWrite(0xa6, DISPLAY_COMMAND); /* set normal display */
  displayWrite(0xa8, DISPLAY_COMMAND); /* set multiplex ratio */
  displayWrite(0x3f, DISPLAY_COMMAND); /* 1/64 */
  displayWrite(0xad, DISPLAY_COMMAND); /* master configuration */
  displayWrite(0x8e, DISPLAY_COMMAND); /* external vcc supply */
  displayWrite(0xc8, DISPLAY_COMMAND); /* set com scan direction */
  displayWrite(0xd3, DISPLAY_COMMAND); /* set display offset */
  displayWrite(0x40, DISPLAY_COMMAND);
  displayWrite(0xd5, DISPLAY_COMMAND); /* set display clock divide/oscillator frequency */
  displayWrite(0xf0, DISPLAY_COMMAND);
  displayWrite(0xD8, DISPLAY_COMMAND); /*set area color mode off */
  displayWrite(0x05, DISPLAY_COMMAND);
  displayWrite(0xD9, DISPLAY_COMMAND);
  displayWrite(0xF1, DISPLAY_COMMAND);
  displayWrite(0xda, DISPLAY_COMMAND); /* set com pin configuartion */
  displayWrite(0x12, DISPLAY_COMMAND);
  displayWrite(0x91, DISPLAY_COMMAND);
  displayWrite(0x3F, DISPLAY_COMMAND);
  displayWrite(0x3F, DISPLAY_COMMAND);
  displayWrite(0x3F, DISPLAY_COMMAND);
  displayWrite(0x3F, DISPLAY_COMMAND);
  displayWrite(0xaf, DISPLAY_COMMAND); /* set display on */
}

void displayWrite(uint8_t x, boolean mode)
{
  digitalWrite(slaveSelectInvPin, LOW);
  digitalWrite(dataCommandInvPin, mode);
  digitalWrite(writeInvPin, LOW);
  displaySetData(x);
  digitalWrite(writeInvPin, HIGH);
  digitalWrite(slaveSelectInvPin, HIGH);
}

static void displaySetData(uint8_t x)
{
  byte y = x;
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(shiftRegDataPin, ((y & 0x80) == 0) ? LOW : HIGH);
    digitalWrite(shiftRegClockPin, HIGH);
    digitalWrite(shiftRegClockPin, LOW);
    y <<= 1;
  }
  digitalWrite(shiftRegClockPin, HIGH);
  digitalWrite(shiftRegClockPin, LOW);
}

void displayClear()
{
  for (int row = 0; row < 8; row++)
  {
    displayGoto(0, row);
    for (int col = 0; col < 132; col++)
    {
      displayWrite(0x00, DISPLAY_DATA);
      displayWrite(0x00, DISPLAY_DATA);
    }
  }
}

void displayGoto(uint8_t x, uint8_t y)
{
  displayWrite(0xb0 | y,                DISPLAY_COMMAND);
  displayWrite(0x00 | ((x + 2) & 0x0f), DISPLAY_COMMAND);
  displayWrite(0x10 | ((x + 2) & 0xf0), DISPLAY_COMMAND);
}

void displaySquare()
{
  for (int i = 0; i < 3; i++)
  {
    displayWrite(0x55, DISPLAY_DATA);
    displayWrite(0xaa, DISPLAY_DATA);
  }
}

void displayBlank()
{
  for (int i = 0; i < 6; i++)
  {
    displayWrite(0x00, DISPLAY_DATA);
  }
}

