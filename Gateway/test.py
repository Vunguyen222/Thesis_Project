# from construct import Struct, Float32l, Int8ub, Int16ul


# ai_gps_frame_t = Struct (
#     "cow_addr" / Int16ul,
#     "latitude" / Float32l,
#     "longitude" / Float32l,
#     "ai_status" / Int8ub,
#     "check_sum" / Int8ub,
# )

# ai_frame_t = Struct (
#     "cow_addr" / Int16ul,
#     "ai_status" / Int8ub,
#     "check_sum" / Int8ub,
# )

# msg = bytes([0x01, 0x00, 0x01, 0x00])

# def handle_lora_msg(msg):
#     ai_gps_frame = None
#     ai_frame = None
#     if len(msg) == 12:
#         ai_gps_frame = ai_gps_frame_t.parse(msg)
#     elif len(msg) == 4:
#         ai_frame = ai_frame_t.parse(msg)
#     else: return

#     cow_id = None
#     cow_addr = ai_gps_frame.cow_addr if ai_gps_frame != None else ai_frame.cow_addr
#     print(cow_addr)

# handle_lora_msg(msg)


import serial
ser = serial.Serial("/dev/ttyUSB1", 9600)

data = bytearray()
data.append(0x00)
data.append(ord('!'))
data.append(0x00)
data.append(0x01)
data.append(ord('#'))
data.append(0x03)

# data.append(0x04)

data.append(0x00)
data.append(ord('!'))
data.append(0x00)
data.append(0x03)
data.append(ord('#'))
data.append(0x01)

data.append(0x00)
data.append(ord('!'))
data.append(0x00)
data.append(0x03)
data.append(ord('#'))

ser.write(data)


"""
    ! 00 01 # 03
"""