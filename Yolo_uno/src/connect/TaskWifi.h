/* TaskWifi.h */
#ifndef INC_TASKWIFI_H_
#define INC_TASKWIFI_H_

#include "globals.h"

// Biến này sẽ cho toàn bộ chương trình biết ta đang ở chế độ nào
extern bool isAPMode; 
extern const int BOOT_BUTTON_PIN;

// Khởi tạo trình quản lý WiFi
extern void initWiFiManager();

// Cố gắng kết nối vào WiFi đã lưu
extern bool connectToSavedWiFi();

// Bắt đầu chế độ Access Point
extern void startAPMode();

// Hàm kiểm tra và kết nối lại (cho chế độ STA)
extern bool Wifi_reconnect();

// HÀM MỚI: Thêm dòng này
extern void checkBootButton();

#endif /* INC_TASKWIFI_H_ */