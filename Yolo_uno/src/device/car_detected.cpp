#include "car_detected.h"

// Mặc định là FALSE (Không hoạt động cho đến khi Camera thấy xe)
bool isParkingSystemActive = false;

void processCarParkingRPC(JSONVar myObject) {
    
    // --- TRƯỜNG HỢP 1: Xử lý RPC (Có "method") ---
    if (myObject.hasOwnProperty("method")) {
        String method = (const char*)myObject["method"];
        if (method == "parking_active") {
            bool active = (bool)myObject["params"];
            isParkingSystemActive = active;
            
            if (active) Serial.println(">>> RPC: System ACTIVATED");
            else Serial.println(">>> RPC: System DEACTIVATED");
        }
    }

    // --- TRƯỜNG HỢP 2: Xử lý SHARED ATTRIBUTE (Không có "method") ---
    // Kiểm tra trực tiếp xem có key "parking_active" không
    else if (myObject.hasOwnProperty("parking_active")) {
        bool active = (bool)myObject["parking_active"];
        
        isParkingSystemActive = active;

        if (active) {
            Serial.println(">>> ATTRIBUTE: System ACTIVATED via Shared Attribute");
        } else {
            Serial.println(">>> ATTRIBUTE: System DEACTIVATED via Shared Attribute");
        }
    }
}