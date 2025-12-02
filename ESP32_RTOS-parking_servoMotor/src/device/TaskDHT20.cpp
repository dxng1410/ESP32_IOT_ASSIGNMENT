#include "TaskDHT20.h"
#define delay_time 10000
#define MY_SCL 11
#define MY_SDA 12

// DHT20 dht20;
float temperature,humidity;
void TaskTemperatureHumidity(void *pvParameters)
{
    while (true)
    {
        getValueDHT20();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}
void getValueDHT20()
{
    if (dht20.read() == DHT20_OK)
    {
        temperature=dht20.getTemperature();
        humidity=dht20.getHumidity();
        publishData("feed_DHT20",String(dht20.getTemperature()) + "-" + String(dht20.getHumidity()));
    }
    else
    {
        Serial.println("Failed to read DHT20 sensor.");
    }
}

void initDHT20()
{
    Wire.begin(MY_SCL, MY_SDA);
    dht20.begin();
    xTaskCreate(TaskTemperatureHumidity, "Task_name", 2048, NULL, 1, NULL);
}


