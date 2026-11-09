# 🏫 Smart Classroom System - IoT Project

Hệ thống **Lớp học thông minh (Smart Classroom)** quản lý ánh sáng, nhiệt độ và tối ưu năng lượng tự động dựa trên nền tảng **ESP32**, giao tiếp **ESP-NOW**, **Modbus RS485**, **IoT Gateway** và server **CoreIOT**. 

Dự án được triển khai theo mô hình phân tán nhiều Node cảm biến, truyền dữ liệu về Hub trung tâm và đồng bộ lên Cloud Dashboard.

---

## 📌 Các Tính năng Chính (Features)

1. **Tự động điều khiển ánh sáng:** Nhận diện sự có mặt của người (PIR Sensor) và đo cường độ ánh sáng (Light Sensor) để bật/tắt đèn khi dưới ngưỡng quy định.
2. **Giám sát & Khống chế nhiệt độ:** Đọc dữ liệu nhiệt độ/độ ẩm phòng qua cảm biến công nghiệp (Modbus RS485) và tự động bật/tắt Quạt/Máy lạnh.
3. **Tiết kiệm năng lượng (Energy Saving):** Tự động chuyển toàn bộ thiết bị về chế độ chờ (Sleep/Save mode) khi không phát hiện chuyển động trong phòng.
4. **4 Chế độ vận hành (Operational Modes):**
   - 📖 **Mode 1 - Study Mode:** Tự động hoàn toàn theo sự có mặt của người, ánh sáng và nhiệt độ phòng.
   - 📽️ **Mode 2 - Present Mode:** Tắt/giảm ánh sáng vùng gần màn chiếu, giữ ánh sáng tự động vùng xa màn chiếu phục vụ giảng dạy/thuyết trình.
   - 💤 **Mode 3 - Save Mode:** Tắt toàn bộ thiết bị tải chính để tiết kiệm điện.
   - 🛠️ **Mode 4 - Manual Mode:** Điều khiển thủ công từng thiết bị trực tiếp từ Dashboard.

---

## 🛠️ Công nghệ Sử dụng (Tech Stack)

* **Phần cứng:** Microcontroller ESP32 (YoloUNO / ESP32-S3), Cảm biến PIR, Cảm biến ánh sáng, Cảm biến nhiệt độ Modbus RS485, Relay module.
* **Giao tiếp:** Modbus RS485 (Chuẩn công nghiệp), ESP-NOW (Không dây băng thông nhanh), Serial USB.
* **Gateway & Cloud:** Python Local Gateway, CoreIOT Platform (ThingsBoard-based RuleChain, Dashboard, OTA Updates).
* **Nền tảng phát triển:** VS Code + **PlatformIO IDE**.

---

## 📁 Quy định Cấu trúc Mã nguồn (Project Structure & Rules)

Để đảm bảo dự án dễ dàng quản lý khi mở rộng nhiều bo mạch (boards) khác nhau, tất cả thành viên **bắt buộc** tuân thủ các quy định phân chia thư mục sau:

```text
├── .pio/                     # Thư mục build tự động của PlatformIO (Ignore trên Git)
├── lib/                      # Các thư viện tự viết hoặc tùy chỉnh cho từng module
│   ├── ModbusSensor/         # Ví dụ: Thư viện đọc cảm biến RS485
│   │   ├── ModbusSensor.h
│   │   └── ModbusSensor.cpp
│   └── EspNowHelper/         # Ví dụ: Thư viện hỗ trợ truyền ESP-NOW
│       ├── EspNowHelper.h
│       └── EspNowHelper.cpp
├── src/                      # Mã nguồn ứng dụng chính (Chia theo Board)
│   ├── node_sensor_rs485/    # Code dành riêng cho Node Cảm biến RS485
│   │   └── main.cpp
│   ├── node_pir_light/       # Code dành riêng cho Node Cảm biến PIR & Light
│   │   └── main.cpp
│   └── gateway_hub_esp32/    # Code dành cho Node Hub/Gateway trung tâm
│       └── main.cpp
├── platformio.ini            # File cấu hình môi trường cho tất cả các Board
└── README.md
```

### 🚨 Quy tắc đóng góp mã nguồn (Coding Conventions):

1. **Cấu trúc Thư viện (`lib/`):**
   * Mọi module/thư viện bổ trợ phải đưa vào thư mục `lib/`.
   * Định dạng bắt buộc: `lib/<Tên_Thư_Viện>/ -> <Tên_Thư_Viện>.h` và `<Tên_Thư_Viện>.cpp` (hoặc `.c`).
   * Không đặt file code rời rạc trực tiếp trong thư mục gốc `lib/`.

2. **Cấu trúc Mã nguồn chính (`src/`):**
   * Mỗi bo mạch (Board) trong hệ thống phải có một thư mục riêng nằm trong `src/` (ví dụ: `src/node_sensor_rs485/`, `src/gateway_hub_esp32/`).
   * File mã nguồn của Board nào phải nằm gọn trong thư mục tương ứng của Board đó.

---

## ⚙️ Hướng dẫn Cài đặt & Setup Môi trường (Environment Setup)

Dự án sử dụng cơ chế **Multi-Environment** của PlatformIO để quản lý và nạp code cho nhiều bo mạch khác nhau trên cùng một Repository.

### 1. Chuẩn bị
* Nền tảng: [Visual Studio Code](https://code.visualstudio.com/)
* Extension: Cài đặt extension **PlatformIO IDE** trong VS Code.

### 2. Cấu hình file `platformio.ini`
Mỗi bo mạch trong dự án được cấu hình thành một `[env:<board_name>]` riêng biệt. Ví dụ:

```ini
[env]
platform = espressif32
framework = arduino
monitor_speed = 115200

; --- Board 1: Sensor Node RS485 ---
[env:node_sensor_rs485]
board = esp32dev
build_src_filter = +<node_sensor_rs485/>

; --- Board 2: Sensor Node PIR & Light ---
[env:node_pir_light]
board = esp32dev
build_src_filter = +<node_pir_light/>

; --- Board 3: Central Gateway Hub ---
[env:gateway_hub_esp32]
board = esp32-s3-devkitc-1
build_src_filter = +<gateway_hub_esp32/>
```

### 3. Cách chọn Environment và Build/Flash Code
1. Mở dự án trong VS Code.
2. Tìm thanh công cụ PlatformIO dưới đáy màn hình (Status Bar) hoặc biểu tượng **PlatformIO Alien** ở cột bên trái.
3. Nhấp vào mục **Project Environment Switcher** và chọn đúng `env` bạn muốn làm việc:
   * Chọn `env:node_sensor_rs485` nếu đang code cho Node cảm biến nhiệt độ.
   * Chọn `env:gateway_hub_esp32` nếu đang làm việc với Node Hub trung tâm.
4. Chọn **Build** (`Ctrl + Alt + B`) hoặc **Upload** (`Ctrl + Alt + U`) để nạp code xuống bo mạch tương ứng.

---

## 👥 Phân công Nhiệm vụ (Team Workflow)

* Trước khi tạo Pull Request mới, hãy đảm bảo code đã được Build thành công trên `env` tương ứng.
* Không commit thư mục tạm `.pio/` hay `.vscode/` lên Git repository.
* Ghi rõ tên Node/Board được cập nhật trong commit message (ví dụ: `feat(node_rs485): add modbus read routine`).

---
*Dự án thuộc Bài tập lớn môn Internet of Things (IoT) - Trường Đại học Bách Khoa - ĐHQG TP.HCM.*
