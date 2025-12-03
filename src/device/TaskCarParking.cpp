#include "TaskCarParking.h"
#include "car_detected.h"
// Biến lưu trạng thái cổng (true = đang mở, false = đang đóng)
bool isGateOpen = false;

// --- 1. HÀM ĐỌC CẢM BIẾN (Dựa trên UltrasonicSensor.cpp) ---
float getUltrasonicDistance() {
    // Tạo xung Trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Đo độ rộng xung Echo (Timeout 30000us ~ 5 mét)
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    // Tính khoảng cách (cm)
    if (duration == 0) return -1; // Lỗi hoặc không có vật cản
    return duration * 0.0343 / 2.0;
}

// --- 2. HÀM ĐIỀU KHIỂN MOTOR (Dựa trên TaskMiniMotor.cpp) ---
void motorStop() {
    digitalWrite(MOTOR_1_PIN, LOW);
    digitalWrite(MOTOR_2_PIN, LOW); // Tắt cả 2 chân để dừng
    Serial.println("Motor: STOP");
}

void motorOpenGate() {
    // Logic quay thuận (Forward) để mở cổng
    // MOTOR_1 HIGH, MOTOR_2 PWM (Low side)
    Serial.println("Motor: OPENING (Forward)...");
    digitalWrite(MOTOR_1_PIN, HIGH);
    analogWrite(MOTOR_2_PIN, 255 - MOTOR_SPEED); 
}

void motorCloseGate() {
    // Logic quay nghịch (Reverse) để đóng cổng
    // Đảo chiều dòng điện: MOTOR_1 LOW, MOTOR_2 HIGH (PWM)
    Serial.println("Motor: CLOSING (Reverse)...");
    digitalWrite(MOTOR_1_PIN, LOW);
    analogWrite(MOTOR_2_PIN, MOTOR_SPEED);
}

// --- 3. TASK CHÍNH ---
void TaskCarParking(void *pvParameters) {
    Serial.println("--- Task Car Parking Started (DC Motor Version) ---");
    
    // Đảm bảo dừng motor khi khởi động
    motorStop();

    while (true) {
        if(isParkingSystemActive == false) {
            motorStop();
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }
        float distance = getUltrasonicDistance();

        // In ra Serial để kiểm tra (Debug)
        if (distance > 0) {
            Serial.print("System Active | Distance: ");
            Serial.println(distance);
        }

        // --- LOGIC ĐIỀU KHIỂN ---
        if (distance > 0 && distance < PARKING_DIST_THRESHOLD) {
            // PHÁT HIỆN XE
            if (!isGateOpen) {
                Serial.println(">>> CAR DETECTED! Opening Gate...");
                motorOpenGate();
                vTaskDelay(GATE_ACTION_TIME / portTICK_PERIOD_MS); // Quay trong 1s
                motorStop(); // Dừng lại sau khi mở
                isGateOpen = true;
            }
        } 
        else {
            // KHÔNG CÓ XE (hoặc xe đã đi)
            if (isGateOpen) {
                // Đợi một chút cho an toàn (debounce) nếu cần
                vTaskDelay(1000 / portTICK_PERIOD_MS); 
                
                // Kiểm tra lại lần nữa cho chắc chắn trước khi đóng
                float confirmDist = getUltrasonicDistance();
                if (confirmDist > PARKING_DIST_THRESHOLD || confirmDist == -1) {
                    Serial.println(">>> CAR LEFT! Closing Gate...");
                    motorOpenGate();
                    vTaskDelay(GATE_ACTION_TIME / portTICK_PERIOD_MS); // Quay ngược lại 1s
                    motorStop(); // Dừng lại sau khi đóng
                    isGateOpen = false;
                }
            }
        }

        vTaskDelay(CHECK_INTERVAL / portTICK_PERIOD_MS);
    }
}

// --- 4. HÀM KHỞI TẠO ---
void initCarParking() {
    Serial.println("Initializing Car Parking (Ultrasonic + DC Motor)...");

    // Cấu hình chân Motor
    pinMode(MOTOR_1_PIN, OUTPUT);
    pinMode(MOTOR_2_PIN, OUTPUT);
    
    // Cấu hình chân Ultrasonic
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Dừng motor ban đầu
    motorStop();

    // Tạo Task
    xTaskCreate(TaskCarParking, "TaskCarParking", 4096, NULL, 1, NULL);
}