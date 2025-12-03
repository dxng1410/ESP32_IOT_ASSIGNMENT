#ifndef LCD_H
#define LCD_H

#include "globals.h"
extern void initLCD();
extern void displayTempHumidity(void *pvParameters);
// extern void displayADCValues();
// extern void displayDistance();
extern void printlcd();
#endif /* LCD_H */