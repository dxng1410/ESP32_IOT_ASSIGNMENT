
#include "TaskMQTT.h"

#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT 1883

String IO_USERNAME = "dxng1410";
String IO_KEY = "aio_tIgW65jkg9iAtsU7m1fGE8bmq3PC";

WiFiClient adafruitWiFiClient; // Đổi tên cho Adafruit
PubSubClient adafruitClient(adafruitWiFiClient); // Đổi tên cho Adafruit

void callback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    if (strcmp(topic, (String(IO_USERNAME) + "/feeds/feed_2").c_str()) == 0)
    {
        Serial.println(message);
    }
    else if (strcmp(topic, (String(IO_USERNAME) + "/feeds/feed_3").c_str()) == 0)
    {
        Serial.println(message);
    }
}

void publishData(String feed, String data)
{
    String topic = String(IO_USERNAME) + "/feeds/" + feed;
    if (adafruitClient.connected())
    {
        adafruitClient.publish(topic.c_str(), data.c_str());
    }
}

void InitMQTT()
{
    Serial.println("Connecting to MQTT...");
    String clientId = "ESP32Client" + String(random(0, 1000));
    if (adafruitClient.connect(clientId.c_str(), IO_USERNAME.c_str(), IO_KEY.c_str()))
    {
        Serial.println("MQTT Connected");
        adafruitClient.subscribe((String(IO_USERNAME) + "/feeds/feed_2").c_str());
        adafruitClient.subscribe((String(IO_USERNAME) + "/feeds/feed_3").c_str());

        String data = "hello";
        publishData("hello", data);
        Serial.println("Start");
        publishData("webseverIP",WiFi.localIP().toString());
    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(adafruitClient.state());
        delay(1000);
    }
}

void reconnectMQTT()
{
    if (adafruitClient.connected())
    {
        adafruitClient.loop();
    }
    else
    {
        InitMQTT();
    }
}

void initMQTT()
{
    adafruitClient.setServer(MQTT_SERVER, MQTT_PORT);
    adafruitClient.setCallback(callback);
}