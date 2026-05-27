# Smart Attendance and Real-Time Telemetry System

## 1. Giới thiệu

Đây là đồ án **hệ thống chấm công thông minh bằng thẻ RFID** sử dụng vi điều khiển **STM32F103C8T6** lập trình bare-metal. Hệ thống đọc UID từ thẻ RFID, hiển thị trạng thái trên màn hình LCD 2004 I2C, giao tiếp với ứng dụng desktop trên máy tính qua UART để kiểm tra thẻ đã đăng ký hay chưa, đồng thời lưu lịch sử chấm công.

Dự án hiện tại gồm 2 phần chính:

```text
1. Firmware STM32 bare-metal
2. Desktop App Python Tkinter + pyserial
```

Trong các phiên bản tiếp theo, dự án sẽ được phát triển thêm phần **thiết kế PCB** để thay thế breadboard/jumper bằng mạch phần cứng hoàn chỉnh.

---

## 2. Mục tiêu đồ án

- Đọc UID thẻ RFID bằng module MFRC522.
- Hiển thị UID và trạng thái thẻ trên LCD 2004 I2C.
- Gửi UID từ STM32 lên máy tính qua UART.
- App máy tính kiểm tra UID trong danh sách đã đăng ký.
- App phản hồi kết quả về STM32.
- STM32 hiển thị kết quả lên LCD.
- App lưu danh sách thẻ và lịch sử quẹt thẻ bằng file CSV.
- Hướng tới thiết kế PCB riêng cho toàn bộ hệ thống.

---

## 3. Phần cứng sử dụng

| Linh kiện | Chức năng |
|---|---|
| STM32F103C8T6 | Vi điều khiển trung tâm |
| MFRC522 RFID | Đọc thẻ RFID 13.56 MHz |
| LCD 2004 I2C PCF8574 | Hiển thị trạng thái |
| USB-TTL | Giao tiếp UART với máy tính |
| Thẻ MIFARE/13.56 MHz | Thẻ chấm công |
| Nguồn 3.3V/5V | Cấp nguồn hệ thống |

---

## 4. Kết nối phần cứng hiện tại

### 4.1. MFRC522 với STM32 qua SPI1

| MFRC522 | STM32F103 |
|---|---|
| SDA/SS | PA4 |
| SCK | PA5 |
| MOSI | PA7 |
| MISO | PA6 |
| RST | PA0 |
| 3.3V | 3.3V |
| GND | GND |

> Lưu ý: MFRC522 dùng mức logic 3.3V, không cấp 5V trực tiếp cho module.

### 4.2. LCD 2004 I2C với STM32

| LCD I2C | STM32F103 |
|---|---|
| SCL | PB6 |
| SDA | PB7 |
| VCC | 3.3V hoặc 5V tùy module |
| GND | GND |

Địa chỉ I2C thường dùng:

```c
#define LCD_I2C_ADDR 0x27
```

Nếu không hoạt động, có thể thử:

```c
#define LCD_I2C_ADDR 0x3F
```

### 4.3. UART với máy tính

| USB-TTL | STM32F103 |
|---|---|
| RX | PA9 / USART1_TX |
| TX | PA10 / USART1_RX |
| GND | GND |

Cấu hình UART:

```text
Baudrate: 9600
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

---

## 5. Kiến trúc hệ thống

```text
+-------------+        SPI        +----------+
| STM32F103   | <---------------> | MFRC522  |
|             |                   +----------+
|             |
|             |        I2C        +---------------+
|             | <---------------> | LCD 2004 I2C  |
|             |                   +---------------+
|             |
|             |       UART        +----------------+
|             | <---------------> | Desktop App PC |
+-------------+                   +----------------+
```

Luồng hoạt động:

```text
Quẹt thẻ RFID
↓
MFRC522 đọc UID
↓
STM32 nhận UID
↓
STM32 hiển thị UID lên LCD
↓
STM32 gửi UID lên PC qua UART
↓
App PC kiểm tra UID trong cards.csv
↓
App phản hồi Known / Unknown / Added
↓
STM32 hiển thị kết quả lên LCD
```

---

## 6. Cấu trúc firmware STM32

Cấu trúc thư mục đề xuất:

```text
Smart_Attendance/
├── Inc/
│   ├── main.h
│   ├── MFRC522.h
│   ├── I2C_LCD.h
│   └── UART.h
├── Src/
│   ├── main.c
│   ├── MFRC522.c
│   ├── I2C_LCD.c
│   └── UART.c
├── startup.c
├── stm32f103c8t6.ld
└── Makefile
```

### 6.1. Các module chính

| File | Chức năng |
|---|---|
| `main.c` | Khởi tạo hệ thống, xử lý luồng chính |
| `MFRC522.c/.h` | Giao tiếp RFID qua SPI |
| `I2C_LCD.c/.h` | Điều khiển LCD 2004 I2C |
| `UART.c/.h` | Gửi/nhận dữ liệu với máy tính |
| `main.h` | Định nghĩa thanh ghi và địa chỉ ngoại vi |
| `startup.c` | Vector table và reset handler |
| `stm32f103c8t6.ld` | Linker script |

---

## 7. Giao thức UART giữa STM32 và app PC

### 7.1. STM32 gửi lên PC

Khi đọc được thẻ, STM32 gửi:

```text
CARD:<UID>\r\n
```

Ví dụ:

```text
CARD:50C7E85F
```

Trong đó UID là 4 byte UID chính của thẻ, không bao gồm byte BCC.

### 7.2. PC phản hồi về STM32

Nếu thẻ đã đăng ký:

```text
Known:<Tên người dùng>\n
```

Ví dụ:

```text
Known:Thanh Tung
```

Nếu thẻ chưa đăng ký:

```text
Unknown\n
```

Nếu app vừa đăng ký thẻ mới:

```text
Added:<Tên người dùng>\n
```

Nếu có lỗi:

```text
Error\n
```

> Hai bên cần thống nhất chữ hoa/thường. App hiện tại xử lý dữ liệu nhận từ STM32 bằng cách chuyển sang chữ hoa khi kiểm tra `CARD:`, nhưng phản hồi về STM32 đang dùng dạng `Known:`, `Unknown`, `Added:`.

---

## 8. Desktop App

Desktop app được viết bằng:

```text
Python 3
Tkinter
pyserial
csv
```

Cấu trúc app:

```text
SmartAttendanceApp/
├── app.py
├── requirements.txt
├── README.md
└── data/
    ├── cards.csv
    └── attendance_log.csv
```

### 8.1. Chức năng app

- Chọn COM port.
- Chọn baudrate.
- Connect/Disconnect UART.
- Nhận UID từ STM32.
- Kiểm tra UID trong `cards.csv`.
- Hiển thị thông tin người đã đăng ký.
- Đăng ký thẻ mới nếu UID chưa tồn tại.
- Ghi lịch sử quẹt thẻ vào `attendance_log.csv`.
- Gửi phản hồi về STM32.

### 8.2. File `cards.csv`

Cấu trúc:

```csv
uid,name,student_id,class_name,registered_at,note
50C7E85F,Thanh Tung,SV001,Embedded,2026-05-27 14:30:00,
```

### 8.3. File `attendance_log.csv`

Cấu trúc:

```csv
uid,name,student_id,class_name,time,status
50C7E85F,Thanh Tung,SV001,Embedded,2026-05-27 14:35:10,Registered
A1B2C3D4,,,,2026-05-27 14:40:00,Unknown
```

---

## 9. Cách build firmware STM32

Yêu cầu:

```text
arm-none-eabi-gcc
make
OpenOCD
ST-Link
```

Build:

```bash
make clean
make
```

Nếu build thành công sẽ tạo:

```text
main.elf
main.bin
main.map
```

Nạp/debug bằng VS Code + Cortex-Debug hoặc OpenOCD.

---

## 10. Cách chạy desktop app

Vào thư mục app:

```bash
cd SmartAttendanceApp
```

Tạo môi trường ảo nếu cần:

```bash
python -m venv .venv
```

Windows PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Linux:

```bash
source .venv/bin/activate
```

Cài thư viện:

```bash
pip install -r requirements.txt
```

Chạy app:

```bash
python app.py
```

Nếu không dùng môi trường ảo trên Ubuntu:

```bash
sudo apt install python3-tk python3-serial
python3 app.py
```

---

## 11. Trạng thái hiện tại của đồ án

Các phần đã hoàn thành:

- Cấu hình clock STM32 bare-metal 36 MHz.
- Giao tiếp SPI với MFRC522.
- Đọc UID thẻ RFID.
- Giao tiếp I2C với LCD 2004 PCF8574.
- Hiển thị ký tự và UID lên LCD.
- Giao tiếp UART với máy tính.
- Gửi UID từ STM32 lên app PC.
- App PC nhận UID, kiểm tra thẻ và lưu CSV.
- App PC gửi phản hồi về STM32.

Các phần đang/ sẽ phát triển:

- Hoàn thiện xử lý `Added:` sau khi đăng ký thẻ mới.
- Thêm timeout cho hàm nhận UART để tránh STM32 chờ vô hạn.
- Thiết kế PCB cho dự án.
- Làm vỏ hộp hoàn chỉnh.
- Tối ưu giao diện app và dữ liệu chấm công.

---

## 12. Dự định thiết kế PCB

Mục tiêu PCB là tích hợp các kết nối chính của hệ thống vào một board gọn, ổn định và dễ lắp đặt hơn so với breadboard.

### 12.1. Khối chức năng trên PCB

PCB dự kiến gồm các khối:

```text
1. Khối vi điều khiển STM32F103C8T6
2. Khối RFID MFRC522
3. Khối LCD 2004 I2C
4. Khối UART/USB-TTL
5. Khối nguồn 3.3V và 5V
6. Khối debug/nạp ST-Link
7. Header mở rộng nếu cần
```

### 12.2. Các connector nên có

| Connector | Chức năng |
|---|---|
| Header MFRC522 | Kết nối module RFID |
| Header LCD I2C | Kết nối LCD 2004 I2C |
| Header UART | Kết nối USB-TTL |
| Header SWD | Nạp/debug STM32 bằng ST-Link |
| Header nguồn | Cấp nguồn ngoài |
| Header mở rộng | Dành cho nâng cấp sau |

### 12.3. Header SWD nên đưa ra PCB

Nên có tối thiểu:

```text
SWDIO
SWCLK
3.3V
GND
NRST
```

### 12.4. Nguồn

Dự kiến PCB cần:

```text
5V input
3.3V regulator cho STM32 và MFRC522
GND chung cho toàn hệ thống
```

Lưu ý:

- STM32F103 và MFRC522 dùng 3.3V.
- LCD I2C có thể dùng 5V hoặc 3.3V tùy module.
- Nếu LCD chạy 5V, cần chú ý pull-up SDA/SCL có thể kéo lên 5V.
- Nên cân nhắc level shifter I2C nếu dùng LCD 5V.

### 12.5. Layout PCB cần chú ý

- Đặt tụ decoupling 100 nF gần chân nguồn STM32.
- Đặt tụ bulk 10 uF hoặc 47 uF gần đầu vào nguồn.
- Đường SPI tới MFRC522 nên ngắn và gọn.
- Không đặt đường tín hiệu tốc độ cao chạy vòng quanh anten RFID.
- GND nên đi chắc, ưu tiên ground plane.
- Header LCD nên bố trí dễ cắm dây.
- Header UART nên ghi rõ TX/RX/GND trên silkscreen.
- Header SWD nên đặt ở mép board để dễ cắm ST-Link.
- Ghi rõ mức điện áp trên PCB: 3V3, 5V, GND.

### 12.6. Checklist trước khi làm PCB

Trước khi vẽ PCB cần chốt:

- Dùng STM32F103C8T6 dạng module Blue Pill hay chip STM32 rời?
- Dùng MFRC522 dạng module hay tích hợp luôn mạch RFID?
- LCD dùng 3.3V hay 5V?
- Có dùng level shifter I2C không?
- Nguồn cấp chính là USB 5V hay adapter ngoài?
- Có cần nút reset, LED trạng thái, buzzer không?
- Kích thước board mong muốn?
- Dùng 1 lớp, 2 lớp hay 4 lớp?

Khuyến nghị giai đoạn đầu:

```text
Thiết kế PCB dạng carrier board:
- STM32 Blue Pill cắm lên board
- MFRC522 cắm qua header
- LCD I2C cắm qua header
- USB-TTL cắm qua header
```

Cách này dễ sửa lỗi hơn so với việc tích hợp toàn bộ chip rời ngay từ đầu.

---

## 13. Hướng phát triển tiếp theo

- Thêm buzzer báo thành công/thất bại.
- Thêm RTC để STM32 tự lưu thời gian nếu không có PC.
- Thêm EEPROM/Flash lưu UID tạm thời trên STM32.
- Thêm chế độ offline.
- Thêm WiFi/ESP8266 hoặc ESP32 để gửi dữ liệu lên server.
- Thiết kế PCB version 1.
- Thiết kế vỏ hộp bằng 3D print hoặc mica.
- Xuất báo cáo thống kê từ file CSV.

---

## 14. Ghi chú Git

Nên thêm `.gitignore` để tránh commit file build và cấu hình máy cá nhân:

```gitignore
*.o
*.elf
*.bin
*.map
.vscode/
.venv/
__pycache__/
dist/
build/
```

Các file nên commit:

```text
Src/
Inc/
Makefile
startup.c
stm32f103c8t6.ld
SmartAttendanceApp/app.py
SmartAttendanceApp/requirements.txt
SmartAttendanceApp/README.md
```

Các file không nên commit:

```text
*.o
*.elf
*.bin
*.map
.vscode/
.venv/
```

---

## 15. Tác giả

```text
Phan Nguyễn Thanh Tùng
Project: Smart Attendance and Real-Time Telemetry System
```
