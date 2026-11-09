#include <Arduino.h>
#include <ArduinoJson.h> // Nhớ cài thư viện ArduinoJson (v6 hoặc v7) trong Library Manager
#ifdef USE_RS485
#include "Read_Temp_Hum.h"
#endif
#include <string.h>
#define PIR_SENSOR GPIO_NUM_8
#define Light_SENSOR GPIO_NUM_1 
#define LIGHT_THRESHOLD 2000
#define TimeSpace 5000UL // tạm thời là 5s để debug
// 1. TẠO STRUCT CHỨA DỮ LIỆU TỔNG HỢP (Không chứa PIR để an toàn cho Ngắt)
struct SystemData {

    float temperature;
    float humidity;
    int lightValue;
    bool isDark;
    bool rs485_ok;
};

// Khởi tạo biến toàn cục và Mutex
SystemData globalData = {0, 0, 0, false, false};
SemaphoreHandle_t dataMutex;

// Biến cờ cho ngắt PIR (Dùng volatile để chống trễ bộ nhớ, không cần dùng Mutex)
volatile bool isMotionDetected = false;
uint32_t lastMotionTime = 0;
#ifdef USE_RS485
// Cấu hình RS485
RS485_SENSOR_TO_UART mySensor(4, 9600, 6, 7, 2, &Serial1);
#endif
// HÀM NGẮT (ISR)
void IRAM_ATTR detectMotion() {
    isMotionDetected = true; // Chỉ bật cờ, không xử lý logic phức tạp ở đây
}

// HÀM XỬ LÝ ÁNH SÁNG
int digitalProcessign(uint16_t PIN_OF_SENSOR) {
    int sum = 0;
    for(int i = 0; i < 20; i++){
        sum += analogRead(PIN_OF_SENSOR);
        vTaskDelay(pdMS_TO_TICKS(5)); // Thay delay() bằng vTaskDelay cho đúng chuẩn RTOS
    }
    return sum / 20;
}

#ifdef USE_RS485
// ---------------------------------------------------------
// LUỒNG 1: CHUYÊN XỬ LÝ RS485 (Nặng, Cần chờ lâu)
// ---------------------------------------------------------
void TaskRS485(void *pvParameters) {
    for (;;) {
      //  Serial.println("[Task 1 - RS485] Dang gui lenh doc Modbus...");
        mySensor.ReadSensor(); 
        
        DataFormSensor hum = mySensor.GetData(0);
        DataFormSensor temp = mySensor.GetData(1);

        // Khóa Mutex để cập nhật dữ liệu an toàn
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            if(hum.flag == 1 && temp.flag == 1) {
                globalData.temperature = temp.Data;
                globalData.humidity = hum.Data;
                globalData.rs485_ok = true;
            } else {
                globalData.rs485_ok = false;
            }
            xSemaphoreGive(dataMutex); // Mở khóa
        }
        
       // Serial.println("[Task 1 - RS485] Doc xong! Ngu 2 giay...");
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}
#endif
// ---------------------------------------------------------
// LUỒNG 2: CHUYÊN XỬ LÝ CẢM BIẾN TƯƠNG TỰ (Nhẹ, Chạy nhanh)
// ---------------------------------------------------------
void TaskLight(void *pvParameters) {
    for (;;) {
        int lightVal = digitalProcessign(Light_SENSOR);
        bool dark = (lightVal < LIGHT_THRESHOLD);

        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            globalData.lightValue = lightVal;
            globalData.isDark = dark;
            xSemaphoreGive(dataMutex);
        }
        
        // Quét liên tục mỗi 500ms, không in ra đây để tránh rác màn hình
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// ---------------------------------------------------------
// HÀM SETUP
// ---------------------------------------------------------
void setup() {
    Serial.begin(115200); 
    pinMode(Light_SENSOR, INPUT);
    pinMode(PIR_SENSOR, INPUT);
    
    Serial1.begin(9600, SERIAL_8N1, 7, 6); 
    attachInterrupt(digitalPinToInterrupt(PIR_SENSOR), detectMotion, RISING);

    Serial.println("\n--- He Thong Khoi Dong Da Luong (FreeRTOS) ---");

    // Khởi tạo Mutex
    dataMutex = xSemaphoreCreateMutex();

    if (dataMutex != NULL) {
      #ifdef USE_RS485
        // Kích hoạt các luồng chạy trên Core 1
        xTaskCreatePinnedToCore(TaskRS485, "TaskRS485", 4096, NULL, 1, NULL, 1);
      #endif  
        xTaskCreatePinnedToCore(TaskLight, "TaskLight", 2048, NULL, 1, NULL, 1);
    }
}

// ---------------------------------------------------------
// LUỒNG CHÍNH (Vòng lặp loop): TỔNG HỢP VÀ ĐÓNG GÓI JSON
// ---------------------------------------------------------
void loop() {
    SystemData localCopy;
    #ifdef USE_RS485
        int id=1;
    #else
        int id=2;
    #endif
    // 1. Xin quyền copy dữ liệu ra ngoài thật nhanh
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        localCopy = globalData;
        xSemaphoreGive(dataMutex);
    }

    // 2. Xử lý logic giữ trạng thái chuyển động (5 phút)
    if (isMotionDetected) {
        lastMotionTime = millis(); // Cập nhật lại mốc thời gian mới nhất khi có người
        isMotionDetected = false;  // Reset cờ báo để sẵn sàng nhận tín hiệu mới
    }
    
    bool motionState = false;
    // Kiểm tra: Hiện tại trừ đi lúc cuối cùng có người, nếu vẫn nhỏ hơn 5 phút (TimeSpace)
    if ((millis() - lastMotionTime) <= TimeSpace) {
        motionState = true;  // Vẫn coi là đang có người
    } else {
        motionState = false; // Đã quá 5 phút không có ai lướt qua
    }

    // 3. Đóng gói vào JSON
    JsonDocument doc; // Cú pháp của ArduinoJson v7 (Nếu dùng v6 thì là DynamicJsonDocument doc(256);)
    doc["id"] = id;
#ifdef USE_RS485
    doc["rs485_status"] = localCopy.rs485_ok ? "OK" : "ERROR";
    doc["temperature"]  = localCopy.rs485_ok ? localCopy.temperature : -1000;
    doc["humidity"]     = localCopy.rs485_ok ? localCopy.humidity : -1;
#endif    
    doc["light_value"]  = localCopy.lightValue;
    doc["is_dark"]      = localCopy.isDark;
    doc["motion"]       = motionState;

    // Xuất JSON ra chuỗi
    String jsonString;
    serializeJson(doc, jsonString);
    
    // 4. In thông tin ra màn hình để quan sát
    Serial.println("\n=================================");
    Serial.println("[Loop] DU LIEU JSON TONG HOP:");
    Serial.println(jsonString);
    Serial.println("=================================\n");

    // Loop chính chạy mỗi 2 giây, độc lập hoàn toàn với quá trình chờ đọc phần cứng ở các luồng trên
    vTaskDelay(pdMS_TO_TICKS(2000)); 
}