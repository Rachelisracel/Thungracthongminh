# HỆ THỐNG THÙNG RÁC THÔNG MINH IOT (SMART TRASH BIN)

Hệ thống thùng rác thông minh đa năng ứng dụng vi điều khiển ESP32 kết hợp nền tảng IoT, hỗ trợ tự động hóa việc thu gom, giám sát mức rác theo thời gian thực và đóng gói nhiệt an toàn.

---

# 1. CÁC TÍNH NĂNG CỐT LÕI
* Mở/đóng nắp tự động:** Cảm biến hồng ngoại phát hiện vật thể/tay người đưa lại gần, điều khiển Servo MG996R mở nắp góc 90° và tự động đóng sau 4 giây.
* Hút rác sát sàn:** Cảm biến hồng ngoại tại khe đáy nhận diện chổi quét, kích hoạt quạt hút Blower 12V cuốn bụi mịn và rác nhỏ vào khoang chứa trong 5 giây.
* Đo mức rác thời gian thực:** Cảm biến siêu âm HC-SR04 liên tục đo khoảng cách và quy đổi ra dung tích rác (%), bật đèn LED cảnh báo khi mức rác vượt ngưỡng an toàn (>= 90%).
* Niêm phong túi rác tự động:** Nhận lệnh kích hoạt, khóa liên động an toàn (khóa nắp), cấp nguồn dây điện trở Mayso hàn miệng túi trong 3.5 giây, chờ nguội 2 giây rồi tự mở nắp cho người dùng lấy túi rác ra ngoài.

---

# 2. BẢNG ÁNH XẠ CHÂN KẾT NỐI 
| Khối chức năng | Linh kiện phần cứng | Chân kết nối ESP32 | Mô tả tín hiệu |
| :--- | :--- | :--- | :--- |
| **Cảm biến mức rác** | HC-SR04 | **TRIG: D5, ECHO: D18** | Đo thời gian xung phản hồi |
| **Cảm biến mở nắp** | FC-51 (Nắp) | **OUT: D19** | Tín hiệu số (Mức LOW kích hoạt) |
| **Cảm biến hút sàn** | FC-51 (Sàn) | **OUT: D23** | Tín hiệu số (Mức LOW kích hoạt) |
| **Cơ cấu mở nắp** | Servo MG996R | **PWM: D13** | Điều khiển góc quay 0° - 90° |
| **Quạt hút sàn** | Relay 1 / Quạt 12V | **IN: D12** | Đóng/ngắt nguồn quạt hút |
| **Cụm gia nhiệt** | Relay 2 / Mayso 12V | **IN: D14** | Kích hoạt dây nhiệt hàn túi |
| **Đèn cảnh báo** | Relay 3 / LED Vàng | **IN: D27** | Bật khi mức rác đầy >= 90% |

---

# 3. CẤU TRÚC THƯ MỤC DỰ ÁN
Thungracthongminh/
├── .pio/                   # Thư mục build của PlatformIO
├── include/                # Thư viện header tự định nghĩa
├── lib/                    # Thư viện ngoài
├── src/
│   └── main.cpp            # Mã nguồn chính điều khiển hệ thống
├── diagram.json            # Sơ đồ kết nối mạch ảo trên Wokwi
├── wokwi.toml              # Cấu hình nạp firmware cho Wokwi
├── platformio.ini          # Cấu hình môi trường và thư viện PlatformIO
└── README.md               # Tài liệu dự án
