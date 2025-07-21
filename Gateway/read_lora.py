import serial

ser = serial.Serial("/dev/ttyUSB1", 9600)

while True:
	if ser.in_waiting:
		data = ser.read(ser.in_waiting)
		hex_list = [f"{b:02X}" for b in data]
		hex_string = ",".join(hex_list)
		print(f"Hex: {hex_string}")
