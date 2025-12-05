#include "LCD.h"

// #define LCD_ADDRESS 0x27  // Địa chỉ I2C mặc định của LCD
// #define LCD_COLUMNS 16   // Số cột LCD
// #define LCD_ROWS 2      // Số dòng LCD
#define DELAY_TIME 1000 // Đọc mỗi 1 giây
#define TIMEOUT 30000   // Timeout 30ms (~5m)
#define DISPLAY_TIME 2000 // Thời gian hiển thị 5s
#define MY_SCL 11
#define MY_SDA 12
// Định nghĩa các chân ADC
#define ADC_A0 1  // GPIO1
#define ADC_A1 2  // GPIO2
#define ADC_A2 3  // GPIO3
#define ADC_A3 4  // GPIO4
LiquidCrystal_I2C lcd(33, 16, 2);
extern DHT20 dht20;     // Từ TaskDHT20.cpp

// SemaphoreHandle_t xNormalSemaphore;
// SemaphoreHandle_t xWarningSemaphore;
// SemaphoreHandle_t xCriticalSemaphore;
// Biến lưu giá trị ADC
int adcValues[4] = {0, 0, 0, 0};

void printlcd()
{
    static bool showTemp = true;  // Biến static để theo dõi trạng thái hiển thị
    static unsigned long lastToggle = 0;  // Thời điểm chuyển đổi cuối cùng
    unsigned long currentMillis = millis();

    // Kiểm tra xem đã đến lúc chuyển đổi hiển thị chưa
    if (currentMillis - lastToggle >= DISPLAY_TIME) {
        lastToggle = currentMillis;  // Cập nhật thời điểm chuyển đổi
        showTemp = !showTemp;        // Đảo trạng thái hiển thị
        lcd.setCursor(0, 1);
        lcd.print("                ");  // Xóa dòng hiện tại
    }

    lcd.setCursor(0, 1);
    if (showTemp) {
        lcd.printf("Temp: %.2f", temperature);
        lcd.print((char)223);
        lcd.print("C");
    } else {
        lcd.printf("Humid: %.2f %%", humidity);
    }
}

void TaskLCD(void *pvParameters)
{
    lcd.begin();
    lcd.backlight();
    while (1)
    {
       if(xSemaphoreTake(xIdealSemaphore, pdMS_TO_TICKS(10))==pdTRUE){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Ideal");
            printlcd();
            Serial.println("Display Ideal");
        }else if(xSemaphoreTake(xNormalSemaphore, pdMS_TO_TICKS(10))==pdTRUE){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Normal");
            printlcd();
            Serial.println("Display Normal");
        }else if(xSemaphoreTake(xWarningSemaphore, pdMS_TO_TICKS(10))==pdTRUE){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Warning");
            printlcd();
            Serial.println("Display Warning");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void displayTempHumidity(void *pvParameters)
{
   Wire.begin(MY_SDA, MY_SCL);
   Serial.begin(115200);
   dht20.begin();
   while(1){
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();
            if(isnan(temperature) || isnan(humidity)){
                Serial.println("Failed to read from DHT20 sensor!");
                temperature = humidity = -1;
                vTaskDelay(pdMS_TO_TICKS(2000));
                continue;
            }
        xSemaphoreTake(xIdealSemaphore, 0);
        xSemaphoreTake(xNormalSemaphore, 0);
        xSemaphoreTake(xWarningSemaphore, 0);

        if(temperature < 25 && humidity < 50){
            xSemaphoreGive(xIdealSemaphore);   
        }else if(temperature >= 25 && temperature < 35 && humidity >= 50 && humidity < 80){
            xSemaphoreGive(xNormalSemaphore);
        }else{
            xSemaphoreGive(xWarningSemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Print the results
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
    }
}


void initLCD()
{
    xIdealSemaphore = xSemaphoreCreateBinary();
    xNormalSemaphore = xSemaphoreCreateBinary();
    xWarningSemaphore = xSemaphoreCreateBinary();
    if (xNormalSemaphore && xWarningSemaphore && xIdealSemaphore) {
        xTaskCreate(TaskLCD, "Task_LCD", 4096, NULL, 1, NULL);
        xTaskCreate(displayTempHumidity, "LCD_display", 4096, NULL, 1, NULL);
    } else {
        Serial.println("ERROR: Failed to create semaphores!");
    }
}



// void    displayADCValues()
// {
//     // Đọc giá trị từ 4 kênh ADC
//     adcValues[0] = analogRead(ADC_A0);
//     adcValues[1] = analogRead(ADC_A1);
//     adcValues[2] = analogRead(ADC_A2);
//     adcValues[3] = analogRead(ADC_A3);
    
//     // Xóa màn hình
//     lcd.clear();
    
//     // Hiển thị A0 và A1 trên dòng 1
//     lcd.setCursor(0, 0);
//     lcd.print("A0:");
//     lcd.print(adcValues[0]);
//     lcd.setCursor(8, 0);
//     lcd.print("A1:");
//     lcd.print(adcValues[1]);
    
//     // Hiển thị A2 và A3 trên dòng 2
//     lcd.setCursor(0, 1);
//     lcd.print("A2:");
//     lcd.print(adcValues[2]);
//     lcd.setCursor(8, 1);
//     lcd.print("A3:");
//     lcd.print(adcValues[3]);
    
//     Serial.println("LCD: Displaying ADC values");
//     Serial.print("A0: "); Serial.print(adcValues[0]);
//     Serial.print(" | A1: "); Serial.print(adcValues[1]);
//     Serial.print(" | A2: "); Serial.print(adcValues[2]);
//     Serial.print(" | A3: "); Serial.println(adcValues[3]);
// }