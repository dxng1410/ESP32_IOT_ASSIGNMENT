/* Webserver.h */
#ifndef INC_TASKWEBSEVER_H_
#define INC_TASKWEBSEVER_H_

#include "globals.h"
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <DHT20.h>

// Function declarations
extern void InitWebsever();
extern void WebSeverloop();

// LED control functions
extern void setRGBColor(int colorIndex);

// Sensor data functions
extern String getSensorDataHTML();

// GPIO and LED state variables
extern String output10State;
extern String output17State;
extern String ledD13State;
extern String rgbLedState;
extern int currentRGBColor;
extern float ml_inference_result; // Lấy biến từ tinyml sang
// Thêm biến này
extern bool isAPMode; 

#endif /* INC_TASKWEBSEVER_H_ */