#include "TaskCoreIOT.h"
#include "../device/car_detected.h" // <--- THÊM: Để gọi hàm xử lý đỗ xe

// Định nghĩa vị trí nếu chưa có
#ifndef LOCATION_LAT
#define LOCATION_LAT 10.7769
#endif

#ifndef LOCATION_LONG
#define LOCATION_LONG 106.7009
#endif

// Khởi tạo Client
WiFiClient coreIotWiFiClient; 
PubSubClient coreiotClient(coreIotWiFiClient); 

// --- HAM CALLBACK: NHẬN TIN NHẮN TỪ THINGSBOARD ---
void coreiotCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  // Parse JSON tin nhắn
  JSONVar myObject = JSON.parse(message);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  // >>> QUAN TRỌNG: GỌI HÀM XỬ LÝ LOGIC ĐỖ XE <<<
  // Hàm này nằm bên file car_detected.cpp, nó sẽ bật/tắt biến isParkingSystemActive
  processCarParkingRPC(myObject);
  // >>> ------------------------------------- <<<
}

// --- HÀM KẾT NỐI LẠI ---
void reconnectCoreIOT() {
    while (!coreiotClient.connected()) {
        Serial.print("Attempting CoreIOT MQTT connection...");
        
        if (coreiotClient.connect(COREIOT_CLIENT_ID, COREIOT_TOKEN, NULL)) {
            Serial.println("connected!");
            
            // 1. Đăng ký nhận lệnh RPC (Cũ)
            coreiotClient.subscribe("v1/devices/me/rpc/request/+");
            
            // 2. THÊM DÒNG NÀY: Đăng ký nhận thay đổi từ Shared Attributes
            coreiotClient.subscribe("v1/devices/me/attributes"); 
            Serial.println("Subscribed to Attributes & RPC");

        } else {
            Serial.print("failed, rc=");
            Serial.print(coreiotClient.state());
            Serial.println(" retrying in 5 seconds");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

// --- HÀM SETUP ---
void setupCoreIOT() {
    // Chờ có mạng WiFi (dùng Semaphore từ TaskWifi)
    while(1) {
        if (xBinarySemaphoreInternet != NULL && xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
            xSemaphoreGive(xBinarySemaphoreInternet); // Trả lại semaphore
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Cấu hình Server & Port
    coreiotClient.setServer(COREIOT_SERVER, COREIOT_PORT);
    coreiotClient.setCallback(coreiotCallback);
    coreiotClient.setBufferSize(2048); // Tăng buffer để nhận tin nhắn dài nếu cần
    
    Serial.println("CoreIOT MQTT client configured");
}

// --- TASK CHÍNH ---
void taskCoreIOT(void *pvParameters) {
    setupCoreIOT();
    
    // Biến đếm thời gian dùng millis() thay vì vTaskDelayUntil cho logic gửi tin
    unsigned long lastSendTime = 0;
    const unsigned long interval = 5000; // 5 giây

    while(1) {
        if (!coreiotClient.connected()) {
            reconnectCoreIOT();
        }
        
        // Hàm này phải chạy LIÊN TỤC để nhận tin nhắn
        coreiotClient.loop(); 

        // Kiểm tra xem đã đến lúc gửi dữ liệu chưa (Non-blocking)
        unsigned long now = millis();
        if (now - lastSendTime > interval) {
            lastSendTime = now;

            // --- ĐOẠN GỬI DỮ LIỆU ---
            JSONVar telemetry;
            telemetry["temperature"] = temperature;
            telemetry["humidity"] = humidity;
            telemetry["long"] = LOCATION_LONG;
            telemetry["lat"] = LOCATION_LAT;
            telemetry["parking_active"] = isParkingSystemActive; 
            String jsonString = JSON.stringify(telemetry);
            coreiotClient.publish("v1/devices/me/telemetry", (const uint8_t*)jsonString.c_str(), jsonString.length());
            // ------------------------
        }

        // Delay cực ngắn để không chiếm dụng CPU, nhưng vẫn phản hồi nhanh
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void initCoreIOT()
{
    Serial.println("Initializing CoreIOT Task...");
    xTaskCreate(
      taskCoreIOT,
      "CoreIOT_Task",
      6144, // Stack size (tăng lên chút cho an toàn với JSON)
      NULL,
      1,
      NULL
    );
}