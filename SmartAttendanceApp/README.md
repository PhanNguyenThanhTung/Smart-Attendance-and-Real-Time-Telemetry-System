# Smart Attendance System - Desktop App

Đây là ứng dụng desktop quản lý chấm công bằng RFID kết nối qua cổng UART (với STM32F103C8T6), được viết bằng Python (Tkinter + pyserial).

## 1. Cấu trúc dự án

```text
SmartAttendanceApp/
├── app.py                  # Chứa toàn bộ mã nguồn ứng dụng (giao diện, logic, UART)
├── requirements.txt        # Danh sách thư viện cần thiết
├── README.md               # File hướng dẫn
└── data/                   # Thư mục chứa dữ liệu
    ├── cards.csv           # File lưu thông tin thẻ đã đăng ký
    └── attendance_log.csv  # File lưu lịch sử quẹt thẻ
```

*Lưu ý:* Thư mục `data/` và các file CSV sẽ tự động được tạo ra khi bạn chạy ứng dụng lần đầu tiên nếu chúng chưa tồn tại.

## 2. Cài đặt môi trường

### Cài đặt Python
Bạn cần cài đặt **Python 3.x** trên máy tính (Nên dùng Python 3.8 trở lên).
Tải Python tại: [python.org](https://www.python.org/downloads/) (Nhớ tích chọn "Add Python to PATH" khi cài đặt).

### Cài đặt thư viện
Mở Terminal / Command Prompt tại thư mục `SmartAttendanceApp` và chạy lệnh sau để cài đặt thư viện cần thiết:
```bash
pip install -r requirements.txt
```

*(Lệnh trên thực chất sẽ cài đặt thư viện `pyserial` để giao tiếp với COM port).*

## 3. Cách chạy ứng dụng

Chạy lệnh sau tại thư mục `SmartAttendanceApp`:
```bash
python app.py
```

## 4. Hướng dẫn sử dụng

### Kết nối với STM32
1. Cắm module USB-to-TTL đã kết nối với STM32 vào máy tính.
2. Mở ứng dụng, bấm nút **Refresh** trong mục **Connection Setup**.
3. Chọn đúng **COM Port** của USB-to-TTL trong danh sách.
4. Chọn **Baudrate** (mặc định với code của bạn là `9600`).
5. Bấm **Connect**. Nếu hiện chữ `Connected` màu xanh là thành công.

### Luồng nhận dữ liệu
- Khi bạn quẹt thẻ trên MFRC522, STM32 sẽ đọc và gửi chuỗi `Card:<UID>\n` lên máy tính.
- **Nếu thẻ đã đăng ký:**
  - Ứng dụng sẽ hiển thị khu vực quét màu xanh, hiện tên người dùng.
  - Phản hồi về STM32: `Known:<Tên>\n`.
- **Nếu thẻ chưa đăng ký:**
  - Ứng dụng sẽ hiển thị khu vực quét màu vàng.
  - Phản hồi về STM32: `Unknown\n`.
  - Bạn có thể nhập Họ tên, MSSV, Lớp và bấm **Register This Card**.
  - Sau khi đăng ký thành công, ứng dụng phản hồi tiếp về STM32: `Added:<Tên>\n`.

### Test ứng dụng không cần STM32
Nếu bạn chưa gắn STM32 nhưng muốn test thử ứng dụng:
1. Dùng 1 cáp USB-to-TTL khác hoặc một phần mềm tạo COM ảo (như com0com, Virtual Serial Port Driver).
2. Dùng phần mềm Terminal (như Hercules, XCTU, hoặc Serial Monitor của Arduino).
3. Gõ gửi chuỗi `CARD:50C7E85F\n` hoặc `Card:50C7E85F\n` vào cổng COM.
4. Xem phản hồi của ứng dụng trên Terminal và giao diện.

## 5. Đóng gói ứng dụng thành file .exe (Dành cho Windows)

Để người khác có thể chạy mà không cần cài đặt Python, bạn có thể đóng gói file `app.py` thành `app.exe` bằng thư viện `PyInstaller`.

**Bước 1:** Cài đặt PyInstaller
```bash
pip install pyinstaller
```

**Bước 2:** Build ra file exe
Tại thư mục `SmartAttendanceApp`, chạy lệnh:
```bash
pyinstaller --onefile --windowed app.py
```
- `--onefile`: Gộp tất cả thành 1 file exe duy nhất.
- `--windowed` (hoặc `--noconsole`): Ẩn cửa sổ dòng lệnh (console) màu đen ở nền khi chạy app.

**Bước 3:** Lấy file exe
Sau khi build xong, bạn vào thư mục `dist/`, lấy file `app.exe` và copy ra thư mục chính `SmartAttendanceApp`. Thư mục `data/` sẽ tự sinh khi bạn chạy file exe.
