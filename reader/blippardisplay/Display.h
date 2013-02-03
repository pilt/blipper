#ifndef DISPLAY_H
#define DISPLAY_H

#include "Arduino.h"

/* Coordinates
 *
 * Display coordinates are relative to the top left corner of the display.
 */

#define DISPLAY_COMMAND LOW
#define DISPLAY_DATA    HIGH

void displaySetup();
void displayReset();
void displayWrite(uint8_t x, boolean mode);

void displayClear();
void displayGoto(uint8_t x, uint8_t y);
void displaySquare();
void displayBlank();

#endif

