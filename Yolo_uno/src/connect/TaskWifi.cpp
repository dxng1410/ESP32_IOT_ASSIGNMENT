/* TaskWifi.cpp */
#include "TaskWifi.h"

// Tên AP, mật khẩu và tên "bộ nhớ" để lưu trữ
const char* AP_SSID = "ESP32-Config";
const char* AP_PASSWORD = NULL; // NULL = không cần mật khẩu
const char* PREF_NAMESPACE = "wifi-creds";
const char* PREF_KEY_SSID = "ssid";
const char* PREF_KEY_PASS = "pass";

// Chân GPIO 0 là nút BOOT
const int BOOT_BUTTON_PIN = 0; 
bool isAPMode = false;
Preferences preferences;

// Thời gian cần giữ nút BOOT để reset (miligiây)
#define BOOT_HOLD_TIME 3000 
unsigned long bootPressStartTime = 0;


// Hàm bắt đầu chế độ AP
void startAPMode() {
    isAPMode = true;
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println("");
    Serial.println("AP Mode Started.");
    Serial.print("Connect to WiFi: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

// Hàm kết nối vào WiFi đã lưu
bool connectToSavedWiFi() {
    preferences.begin(PREF_NAMESPACE, true); // true = read-only
    String ssid = preferences.getString(PREF_KEY_SSID, "");
    String pass = preferences.getString(PREF_KEY_PASS, "");
    preferences.end();

    if (ssid.length() == 0) {
        Serial.println("No saved WiFi credentials.");
        return false;
    }

    Serial.print("Connecting to AP: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;
        if (attempts > 20) { // Timeout 10 giây
            Serial.println("\nFailed to connect.");
            WiFi.disconnect();
            return false;
        }
    }
    Serial.println("\nConnected to AP");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

/**
 * @brief Khởi tạo WiFi (ĐÃ ĐƠN GIẢN HÓA)
 * Sẽ không còn kiểm tra nút BOOT ở đây nữa.
 */
void initWiFiManager() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    Serial.println("Trying to connect to saved WiFi...");

    if (!connectToSavedWiFi()) {
        // Nếu kết nối thất bại (hoặc chưa có) -> vào AP mode
        Serial.println("Connection failed or no credentials. Starting AP mode.");
        startAPMode();
    } else {
        // Kết nối thành công -> vào STA mode
        Serial.println("Connected to saved WiFi.");
        isAPMode = false;
    }
}

// Hàm kết nối lại
bool Wifi_reconnect() {
    if (isAPMode) {
        return true; 
    }
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    Serial.println("WiFi disconnected. Reconnecting...");
    return connectToSavedWiFi();
}


/**
 * @brief HÀM MỚI: Kiểm tra nút BOOT trong vòng lặp loop()
 * Nếu giữ nút 3 giây, nó sẽ xóa WiFi và khởi động lại.
 */
void checkBootButton() {
    // Đọc trạng thái nút BOOT
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        // Nút đang được nhấn
        if (bootPressStartTime == 0) {
            // Đây là lần đầu phát hiện, ghi lại thời gian
            bootPressStartTime = millis();
            Serial.println("BOOT button pressed. Hold for 3 seconds to reset to AP mode...");
        } else if (millis() - bootPressStartTime > BOOT_HOLD_TIME) {
            // Đã giữ đủ 3 giây
            Serial.println("BOOT button held. Forcing AP mode and clearing saved WiFi...");
            
            // Xóa thông tin WiFi đã lưu
            preferences.begin(PREF_NAMESPACE, false); // false = read-write
            preferences.clear();
            preferences.end();

            Serial.println("WiFi credentials cleared. Restarting...");
            delay(500); // Chờ 0.5 giây để Serial kịp in ra
            ESP.restart(); // Khởi động lại
        }
    } else {
        // Nút không được nhấn (đã thả ra), reset bộ đếm
        if (bootPressStartTime > 0) {
            Serial.println("BOOT button released before reset.");
        }
        bootPressStartTime = 0;
    }
}