# Hướng dẫn build và chạy FlashUart.py trên Windows với Python

## 1. Chạy tool với tham số:

python FlashUart.py --port COM6 --baud 115200 --addr 0x08004000 --bin ./Build/Application.bin

Trong đó:

- `COM6` là cổng serial
- `115200` là baudrate
- `0x08004000` là địa chỉ flash
- `./Build/Application.bin` là file nhị phân
- `5` là timeout mỗi lệnh (giây)

## 2. Lưu ý

- Đảm bảo file Application.bin tồn tại đúng đường dẫn.
- Nếu báo lỗi không mở được cổng COM, hãy kiểm tra driver hoặc quyền truy cập.
