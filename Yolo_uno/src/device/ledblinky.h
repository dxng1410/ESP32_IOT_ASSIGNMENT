#ifndef __LED_BLINKY__
#define __LED_BLINKY__

#include "globals.h"
#define LED_GPIO 48

// Temperature thresholds
// #define TEMP_LOW 25.0    // Below this is considered cold
// #define TEMP_HIGH 28.0   // Above this is considered hot
//                         // Between TEMP_LOW and TEMP_HIGH is normal

// Blink patterns (in milliseconds)
#define BLINK_COLD 2000  // Slow blink for cold
#define BLINK_NORMAL 1000 // Medium blink for normal
#define BLINK_HOT 500    // Fast blink for hot

// extern SemaphoreHandle_t xTempSemaphore = NULL;  // Semaphore for temperature access

void led_blinky(void *pvParameters);
void initLedBlinkPattern();
// void updateBlinkPattern();
void temp_monitor_task(void *pvParameters);

#endif