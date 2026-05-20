

#!/usr/bin/env python3
import argparse
import os
import sys
import time


import serial

READY = b"READY"
ACK = b"ACK"
DONE = b"DONE"
ERR = b"ERR"

BLOCK_SIZE = 1024

MESSAGE_TOKEN = 0x7E

def wait_for_cmd(port, expected, timeout_s):
    """
    Đọc dữ liệu từ UART cho đến khi nhận được chuỗi expected hoặc ERR hoặc hết timeout.
    Trả về True nếu nhận được expected, False nếu nhận ERR hoặc timeout.
    """
    buf = bytearray()
    start = time.time()
    while time.time() - start < timeout_s:
        if port.in_waiting:
            data = port.read(port.in_waiting)
            if data:
                buf += data
                # print(f"RX: {data}")
                if expected in buf:
                    return True
                if ERR in buf:
                    print("Received ERR")
                    return False
        time.sleep(0.01)
        
    print("Timeout waiting for command", flush=True)
    return False


def flash_uart(port_name, baud, addr, bin_path, timeout_s):
    size = os.path.getsize(bin_path)
    if size <= 0:
        raise RuntimeError("empty binary")

    with serial.Serial(port_name, baudrate=baud, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1) as port:
        # port.open()   # Mở lại cổng để gửi dữ liệu
        port.reset_input_buffer()
        port.reset_output_buffer()

        print("Start sending data")

        # Gửi lệnh FLASH <addr> <size>\r\n (chỉ gửi size 1 lần)
        cmd =  "~"+ "FLASH 0x%08X %d\n" % (addr, size) + '~'
        print("TX:", repr(cmd.encode("ascii")))
        port.write(cmd.encode("ascii"))
        time.sleep(0.1)
        

        if not wait_for_cmd(port, READY, timeout_s):
            raise RuntimeError("Did not receive READY")

        port.reset_input_buffer()
        port.reset_output_buffer()
        
        # port.close()  # Đóng cổng để máy chủ bootloader biết là đã gửi xong lệnh và bắt đầu nhận dữ liệu
        

        sent = 0
        block_idx = 0
        print("Start sending data")
        
        # send_bin_raw(port_name, baud, bin_path)
        # time.sleep(0.5)
        try:
            with open(bin_path, "rb") as f:
                while sent < size:
                    # port.open()   # Mở lại cổng để gửi dữ liệu
                    remain = size - sent
                    chunk_size = BLOCK_SIZE if remain > BLOCK_SIZE else remain
                    chunk = f.read(chunk_size)
                    if not chunk:
                        break
                    chunk_len = len(chunk)
                    port.write(chunk)
                    sent += chunk_len
                    block_idx += 1
                    time.sleep(0.5)
                    print(f"Sent {sent}/{size} bytes", end='\r')
                    # got_ack = wait_for_cmd(port, ACK, timeout_s)
                    # if not got_ack:
                    #     raise RuntimeError("No ACK received")
                    # port.close()  # Đóng cổng để máy chủ bootloader biết là đã gửi xong lệnh và bắt đầu nhận dữ liệu
        except Exception as e:
            print(f"ERROR: Cannot open/read binary file '{bin_path}': {e}")
            raise
        
        port.reset_output_buffer()
        
        if sent != size:
            raise RuntimeError("sent size mismatch: %d != %d" % (sent, size))

        if not wait_for_cmd(port, DONE, 10):
            raise RuntimeError("Flashing failed, DONE not received")
        else:
            print("Flashing completed successfully")


# def send_bin_raw(port_name, baud, bin_path):
#     BLOCK_SIZE = 1024
#     size = os.path.getsize(bin_path)
#     with serial.Serial(port_name, baudrate=baud, timeout=0) as port:
#         with open(bin_path, "rb") as f:
#             sent = 0
#             while sent < size:
#                 chunk = f.read(BLOCK_SIZE)
#                 if not chunk:
#                     break
#                 port.write(chunk)
#                 sent += len(chunk)
#                 print(f"Sent {sent}/{size} bytes", end='\r')
#     print("\nDone sending file.")

# def compute_crc(buff, length):
#     crc = 0xFFFFFFFF
#     #print(length)
#     for byte in buff[0:length]:
#     crc = crc ^ (byte)
#     for i in range(32):
#         if(crc & 0x80000000):
#             crc = (crc << 1) ^ 0x04C11DB7
#         else:
#             crc = (crc << 1)
#     return crc & 0xFFFFFFFF

# def send_data_serial(serial_com, data):
#     data = data + (struct.pack('<I',compute_crc(data, len(data))))
#     serial_com.write(data)
    
# def update_flash_mem(serial_com, file_name):
#     WINDOW_SIZE  = 128
#     try:
#         flash_bin_file = open(file_name, 'rb')
#     except:
#         Exception('cannot open the bin file')
#     size = os.path.getsize(file_name)
#     # data token and command
#     data_token = [MESSAGE_TOKEN, BOOT_UPDATE_REQUEST]
#     data_token_bytes = bytes()
#     data_token_bytes = data_token_bytes.join((struct.pack('<'+format, val) for format,val in zip('BB',data_token)))
#     size_copy = size
#     while size > 0:
#         # actual data
#         data_sent = flash_bin_file.read(WINDOW_SIZE)
#         size = size - len(data_sent)
#         data_sent = data_token_bytes + struct.pack('<H',len(data_sent)) + data_sent
#         send_data_serial(serial_com, data_sent)
#         print('Firmware update ' + str(int(100 * (size_copy - size)/size_copy)) +' %' +': ' + int(50 * (size_copy - size)/size_copy) * '#', end = '\r')
#         if read_boot_reply(serial_com) != BOOT_ACK:
#             print("flash update failed ")
#             Exception("flash update failed")
#             break

#     if size == 0:
#         print('Firmware update ' + str(int(100 * (size_copy - size)/size_copy)) +' %' +': ' + \
#             int(20 * (size_copy - size)/size_copy) * '#')
#         print('Firmware update is over')


    
def main():
    parser = argparse.ArgumentParser(description="Flash STM32 bootloader over UART")
    parser.add_argument("--port", required=True, help="Serial port (e.g. COM3)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--addr", type=lambda x: int(x, 0), required=True, help="Flash start address (hex or dec)")
    parser.add_argument("--bin", required=True, help="Path to binary file")
    parser.add_argument("--timeout", type=float, default=5.0, help="Timeout per response in seconds")
    args = parser.parse_args()


    try:
        flash_uart(args.port, args.baud, args.addr, args.bin, args.timeout)
        print("OK")
        return 0
    except Exception as exc:
        print("ERROR:", exc)
        return 1


if __name__ == "__main__":
    sys.exit(main())
