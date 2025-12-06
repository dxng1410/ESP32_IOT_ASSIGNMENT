#include "globals.h"

DHT20 dht20;

SemaphoreHandle_t xTempSemaphore = NULL;
SemaphoreHandle_t xHumidSemaphore = NULL;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

SemaphoreHandle_t xInferenceResultSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t xNormalSemaphore = NULL;
SemaphoreHandle_t xWarningSemaphore = NULL;
SemaphoreHandle_t xIdealSemaphore = NULL;

float glob_inference_result;