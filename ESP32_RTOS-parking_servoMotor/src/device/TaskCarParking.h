#ifndef TASK_CAR_PARKING_H
#define TASK_CAR_PARKING_H

#include "globals.h"

// --- CẤU HÌNH CHÂN (Theo yêu cầu của bạn) ---
#define MOTOR_1_PIN 10  // Chân điều khiển Motor 1
#define MOTOR_2_PIN 17  // Chân điều khiển Motor 2 (Có PWM)
#define MY_SCL 11
#define MY_SDA 12
#define TRIG_PIN    18  // Chân Trigger cảm biến siêu âm
#define ECHO_PIN    21  // Chân Echo cảm biến siêu âm

// --- THÔNG SỐ CẤU HÌNH ---
#define PARKING_DIST_THRESHOLD 10.0 // cm (Khoảng cách phát hiện xe)
#define GATE_ACTION_TIME       1000 // ms (Thời gian quay motor để mở/đóng cổng)
#define MOTOR_SPEED            255  // Tốc độ tối đa (0-255)
#define CHECK_INTERVAL         100  // ms (Chu kỳ quét cảm biến)

// --- FUNCTION PROTOTYPES ---
void initCarParking();
void TaskCarParking(void *pvParameters);

// Các hàm phụ trợ (được tái sử dụng logic từ file cũ của bạn)
float getUltrasonicDistance();
void motorOpenGate();
void motorCloseGate();
void motorStop();

#endif