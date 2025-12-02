#include "ledblinky.h"

// ---- Định nghĩa cấu trúc dữ liệu ----
typedef struct {
    float temperature;
    SemaphoreHandle_t xTempSemaphore;
} TempData;

// ---- Biến toàn cục ----
// extern float glob_temperature;
static TempData tempData;
uint32_t currentBlinkInterval = BLINK_NORMAL;

// ---- Task LED Blinky ----
void led_blinky(void *pvParameters) {
    TempData *data = (TempData *) pvParameters;
    pinMode(LED_GPIO, OUTPUT);

    while (1) {
        float currentTemp = 0.0;

        // Lấy nhiệt độ an toàn bằng semaphore
        if (xSemaphoreTake(data->xTempSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            currentTemp = data->temperature;
            xSemaphoreGive(data->xTempSemaphore);
        }

        String data;
        // Xác định kiểu nháy
        if (currentTemp < 25.0) {
            currentBlinkInterval = BLINK_COLD;
            data = "[LED] Temperature LOW → blinking SLOW";
            Serial.println(data);
        } else if (currentTemp > 28.0) {
            currentBlinkInterval = BLINK_HOT;
            data = "[LED] Temperature HIGH → blinking FAST";
            Serial.println(data);
        } else {
            currentBlinkInterval = BLINK_NORMAL;
            data = "[LED] Temperature NORMAL → blinking NORMAL";
            Serial.println(data);
        }
        publishData("Temperature state",data);

        // Nháy LED
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(currentBlinkInterval));
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(currentBlinkInterval));
    }
}

// ---- Task cập nhật nhiệt độ ----
void temp_monitor_task(void *pvParameters) {
    TempData *data = (TempData *) pvParameters;

    while (1) {
        if (xSemaphoreTake(data->xTempSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
            data->temperature =dht20.getTemperature(); // đọc từ global
            xSemaphoreGive(data->xTempSemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ---- Khởi tạo module LED Blinky ----
void initLedBlinkPattern() {
    tempData.xTempSemaphore = xSemaphoreCreateMutex();
    tempData.temperature = 0.0;

    if (tempData.xTempSemaphore == NULL) {
        Serial.println("[ERROR] Failed to create xTempSemaphore");
        return;
    }

    BaseType_t t1 = xTaskCreate(temp_monitor_task, "Task_Temp_Monitor", 2048, &tempData, 1, NULL);
    BaseType_t t2 = xTaskCreate(led_blinky, "Task_LED_Blinky", 2048, &tempData, 1, NULL);

    if (t1 == pdPASS && t2 == pdPASS)
        Serial.println("[INIT] LED Blinky + Temp Monitor tasks created successfully");
    else
        Serial.println("[ERROR] Failed to create LED tasks");
}