#ifndef TASK_COREIOT_H
#define TASK_COREIOT_H

#include "globals.h"
#include <PubSubClient.h>
// CoreIOT Configuration
#define COREIOT_SERVER "app.coreiot.io"
#define COREIOT_PORT 1883
#define COREIOT_TOKEN "70070OYyZr294feRZtj6"  // Your device token
#define COREIOT_CLIENT_ID "83cb38f0-ce4e-11f0-b529-c7574d2602e9"

// Location coordinates (HCMUT)
#define LOCATION_LAT 10.772175109674038
#define LOCATION_LONG 106.65789107082472

// Function declarations
void setupCoreIOT();
void taskCoreIOT(void *pvParameters);
void reconnectCoreIOT();
void coreiotCallback(char* topic, byte* payload, unsigned int length);
void initCoreIOT();
extern WiFiClient coreIotWiFiClient;
extern PubSubClient coreiotClient;

#endif // TASK_COREIOT_H