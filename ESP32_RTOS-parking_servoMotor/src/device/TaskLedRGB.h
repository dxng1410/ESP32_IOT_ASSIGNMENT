#ifndef INC_LEDRGB_H_
#define INC_LEDRGB_H_

#include "globals.h"

#define MY_SCL 11
#define MY_SDA 12

#define NUM_PIXEL 45
#define PIN_PIXEL 8

extern void initLEDRGB();
void taskLEDRGB(void *pvParameters);
void humid_monitor(void *pvParameters);
#endif /* INC_LEDRGB_H_ */