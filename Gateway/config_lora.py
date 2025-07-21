import serial

ser = serial.Serial("/dev/ttyUSB1", 9600, timeout = 1)

is_config_center = 1

if is_config_center == 1:
    ser.write([0xc0, 0x00, 0x02, 0xff, 0xff]) # address 0xff
    print(ser.read(1000))

    ser.write([0xc0, 0x02, 0x01, 0x70]) # default: 9600 8n1 125bw sf9
    print(ser.read(1000))

    ser.write([0xc0, 0x05, 0x01, 0x03]) # default: channel : 920.6 (0)
    print(ser.read(1000))

    ser.write([0xc0, 0x06, 0x02, 0x11, 0x11]) # key: 1111
    print(ser.read(1000))
else:
    ser.write([0xc0, 0x00, 0x02, 0x00, 0x01]) # address 0x00
    print(ser.read(1000))

    ser.write([0xc0, 0x02, 0x01, 0x70]) # default: 9600 8n1 125bw sf9
    print(ser.read(1000))

    ser.write([0xc0, 0x05, 0x01, 0x03]) # transparent mode
    print(ser.read(1000))

    ser.write([0xc0, 0x04, 0x01, 0x00]) # default: channel : 920.6 (0)
    print(ser.read(1000))

    ser.write([0xc0, 0x06, 0x02, 0x11, 0x11]) # key: 1111
    print(ser.read(1000))






