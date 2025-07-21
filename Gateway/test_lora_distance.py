import serial

ser = serial.Serial("/dev/ttyUSB0", 9600)

# Mở file để ghi, thêm 'a' để ghi tiếp chứ không ghi đè
with open("output.txt", "a") as f:
    while True:
        n_bytes = ser.in_waiting
        if n_bytes > 0:
            bytes_recv = ser.read(n_bytes)
            s = f"Receive {len(bytes_recv)} bytes: {bytes_recv}\n"
            print(s, end='')  # end='' để tránh in thừa dòng trống
            f.write(s)
            f.flush()  # Đảm bảo ghi ngay ra file, không bị giữ trong bộ đệm
