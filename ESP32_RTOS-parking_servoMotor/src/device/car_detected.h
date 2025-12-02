#ifndef CAR_DETECTED_H
#define CAR_DETECTED_H
#include "globals.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
// Biến toàn cục để kiểm soát trạng thái hệ thống
// true = Camera thấy xe -> Cho phép cảm biến hoạt động
// false = Camera không thấy xe -> Vô hiệu hóa cảm biến
extern bool isParkingSystemActive;

// Hàm xử lý lệnh RPC từ ThingsBoard gửi xuống
void processCarParkingRPC(JSONVar myObject);

#endif