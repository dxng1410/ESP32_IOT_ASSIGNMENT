#include "TaskLedRGB.h"

Adafruit_NeoPixel strip(PIN_PIXEL, NUM_PIXEL, NEO_GRB + NEO_KHZ800);

// ---- Cấu trúc dữ liệu dùng chung ----
typedef struct {
    float humidity;
    SemaphoreHandle_t xHumidSemaphore;
} HumidData;

// ---- Biến cục bộ ----
static HumidData humidData;

// ---- Bảng màu ----
uint32_t color[] = {
    strip.Color(255, 0, 0),      //Red
    strip.Color(255, 127, 0),    //Orange
    strip.Color(255, 255, 0),    //Yellow
    strip.Color(0, 255, 0),      //Green
    strip.Color(75, 0, 130),     //Violet
    strip.Color(148, 0, 211),    //Purple
    strip.Color(255, 255, 255)  //White
};

// ---- Task 1: Đọc cảm biến DHT20 ----
void humid_monitor(void *pvParameters) {
    HumidData *data = (HumidData *)pvParameters;

    Wire.begin(11, 12);
    dht20.begin();

    while (1) {
        dht20.read();
        float humidity = dht20.getHumidity();

        if (isnan(humidity)) {
            Serial.println("[HUMID] Failed to read humidity!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        // Cập nhật dữ liệu an toàn bằng semaphore
        if (xSemaphoreTake(data->xHumidSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            data->humidity = humidity;
            xSemaphoreGive(data->xHumidSemaphore);
        }

        Serial.printf("[HUMID] Current humidity: %.1f%%\n", humidity);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ---- Task 2: Hiển thị LED theo độ ẩm ----
void taskLEDRGB(void *pvParameters) {
    HumidData *data = (HumidData *)pvParameters;

    strip.begin();
    strip.setBrightness(100);
    strip.show();

    while (1) {
        float currentHumid = 0.0;

        // Đọc dữ liệu an toàn bằng semaphore
        if (xSemaphoreTake(data->xHumidSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            currentHumid = data->humidity;
            xSemaphoreGive(data->xHumidSemaphore);
        }

        // Đổi màu LED theo mức ẩm
        String data;
        if (currentHumid < 30) {
            strip.fill(color[0]);
            data = "[LED] (RED)"; 
            Serial.println(data);
        }else if (currentHumid < 40) {
            strip.fill(color[1]);
            data = "[LED] (ORANGE)";
            Serial.println(data);
        }else if (currentHumid < 50) {
            strip.fill(color[2]); 
            data = "[LED] (YELLOW)";
            Serial.println(data);
        }else if (currentHumid < 60) {
            strip.fill(color[3]);
            data = "[LED] (GREEN)";
            Serial.println(data);
        }else if (currentHumid < 70) {
            strip.fill(color[4]);
            data = "[LED] (VIOLET)";
            Serial.println(data);
        }else if (currentHumid < 80) {
            strip.fill(color[5]);
            data = "[LED] (PURPLE)";
            Serial.println(data);
        } else {
            strip.fill(color[6]);
            data = "[LED] (WHITE)";
            Serial.println(data);
        }
        publishData("Humidity state",data);
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ---- Khởi tạo module ----
void initLEDRGB() {
    humidData.xHumidSemaphore = xSemaphoreCreateMutex();
    humidData.humidity = 0.0;

    if (humidData.xHumidSemaphore == NULL) {
        Serial.println("[ERROR] Failed to create xHumidSemaphore");
        return;
    }

    BaseType_t t1 = xTaskCreate(humid_monitor, "Task_HumidMonitor", 4096, &humidData, 1, NULL);
    BaseType_t t2 = xTaskCreate(taskLEDRGB, "Task_NeoLED", 4096, &humidData, 1, NULL);

    if (t1 == pdPASS && t2 == pdPASS)
        Serial.println("[INIT] Neo Blinky + Humid Monitor tasks created successfully");
    else
        Serial.println("[ERROR] Failed to create Neo LED tasks");
}