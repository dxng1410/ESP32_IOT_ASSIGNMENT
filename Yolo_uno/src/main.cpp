#include "globals.h"

void setup()
{
  xBinarySemaphoreInternet = xSemaphoreCreateBinary();
  Serial.begin(115200);
  // delay(1000); // Chờ Serial ổn định
  initWiFiManager(); // Hàm này giờ đã đơn giản, không cần chờ 2 giây

  if (isAPMode) {
    Serial.print("   Web Server AP Mode. Connect to WiFi 'ESP32-Config' and go to: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("   Web Server STA Mode. Connected to WiFi. Access at: http://");
    Serial.println(WiFi.localIP());
  }

  if (!isAPMode) {
    xSemaphoreGive(xBinarySemaphoreInternet); 
    initMQTT();  
  } 
  xSemaphoreGive(xInferenceResultSemaphore);
  initDHT20();
  initCoreIOT();
  initLCD();
  initLedBlinkPattern();
  initLEDRGB();
  tinyML();
  InitWebsever();

  Serial.println("--- SETUP COMPLETE ---");
  Serial.println("Webserver is now running. Waiting for connections...");
  Serial.println(">>> HOLD BOOT BUTTON FOR 3 SECONDS TO RESET TO AP MODE <<<");
  initCarParking();
}

void loop()
{
  // THÊM HÀM NÀY VÀO ĐẦU TIÊN
  // Nó sẽ kiểm tra nút BOOT và tự khởi động lại nếu cần
  checkBootButton();

  // Chỉ kiểm tra và kết nối lại MQTT/WiFi nếu ở chế độ STA
  if (!isAPMode) {
    if (!Wifi_reconnect())
    {
      return;
    }
    reconnectMQTT();
  }
  
  // Vòng lặp Webserver luôn chạy
  WebSeverloop();
}