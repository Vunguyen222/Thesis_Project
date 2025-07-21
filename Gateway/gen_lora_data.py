import serial
from construct import Struct, Float32l, Int8ub, Int16ul
import sys

def calculate_checksum(buf, length):
    result = 0
    for i in range(length):
        result ^= buf[i]
    return result

ser = serial.Serial("/dev/pts/5", 9600)

ai_gps_frame_t = Struct (
    "cow_addr" / Int16ul,
    "latitude" / Float32l,
    "longitude" / Float32l,
    "cow_status" / Int8ub,
)

ai_frame_t = Struct (
    "cow_addr" / Int16ul,
    "cow_status" / Int8ub,
)

cow_addr = 1
cow_status = 1
latitude = 10.881820149499562
longitude = 106.79891973499126

data = None

# Kiểm tra tham số từ dòng lệnh
if sys.argv[1] == "gps":
    # Xây dựng dữ liệu GPS
    data = ai_gps_frame_t.build({
        "cow_addr": cow_addr,
        "latitude": latitude,
        "longitude": longitude,
        "cow_status": cow_status
    })
elif sys.argv[1] == "ai":
    # Xây dựng dữ liệu AI
    data = ai_frame_t.build({
        "cow_addr": cow_addr,
        "cow_status": cow_status
    })
else:
    print("Invalid argument. Use 'gps' or 'ai'.")
    sys.exit(1)

data = b'!' + data + b'#'
check_sum = calculate_checksum(data, len(data))
data += bytes([check_sum])

# Gửi dữ liệu qua UART
ser.write(data)
print(f"Data sent: {data}")