/* globals.h */
#ifndef GLOBALS_H
#define GLOBALS_H

// Include libraries
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <DHT20.h>
// #include <FreeRTOS.h>
// #include <semphr.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "freertos/FreeRTOS.h"   // Thư viện FreeRTOS gốc
#include "freertos/task.h"       // Để dùng xTaskCreate
#include "freertos/semphr.h"     // Để dùng Semaphore (sửa lỗi hiện tại của bạn)
#include "freertos/queue.h"      // (Thêm phòng hờ) Để dùng Queue
#include <Preferences.h> // <-- THÊM DÒNG NÀY



extern Preferences preferences;

#include <PIR.h>
#include <PubSubClient.h>
#include <I2C_LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
// Pin Definitions
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ElegantOTA.h>
#include <ESP_Google_Sheet_Client.h>
// #include <GS_SDHelper.h>
// Include connection modules
#include "../src/connect/TaskWifi.h"
#include "../src/connect/TaskMQTT.h"
#include "../src/connect/Webserver.h"
#include "device/ledblinky.h"
#include "../src/connect/TaskCoreIOT.h"
// Include device modules
#include "../src/device/TaskDHT20.h"
#include "../src/device/LCD.h"
#include "../src/device/TaskLedRGB.h"
#include "../src/device/tinyml.h"
#include "../src/device/TaskCarParking.h"
#include "../src/device/car_detected.h"

extern SemaphoreHandle_t xNormalSemaphore;
extern SemaphoreHandle_t xWarningSemaphore ;
extern SemaphoreHandle_t xIdealSemaphore ;
extern SemaphoreHandle_t xTempSemaphore;
extern SemaphoreHandle_t xHumidSemaphore;
extern SemaphoreHandle_t xInferenceResultSemaphore;
extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASSWORD;
extern String wifi_password;
extern String wifi_ssid;
extern String ssid;
extern String password;
extern DHT20 dht20;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;
extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern float glob_inference_result;

#endif