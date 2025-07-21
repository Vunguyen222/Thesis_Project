import sys
from serial import SerialException, Serial

class Lora:
    """
        This class will handle lora communication

        Attributes:
            lora_ser (serial.Serial): UART instance use to receive data from lora 
            lora_msg (str): A buffer store data from lora

    """
    def __init__(self, lora_port: str):
        self.handle_lora_msg = None

        # init serial instance
        self.lora_ser = Serial(port = lora_port, baudrate = 9600)

        # init attributes use to handle message from Lora
        # this attribute use to store message from Lora
        self.lora_msg = []
        self.is_begin = False
        self.FSM_READ_INIT = 0
        self.FSM_READ_BEGIN = 1
        self.FSM_READ_END = 2
        self.fsm_read_status = self.FSM_READ_INIT

    def calculate_checksum(self, buf, length):
        result = 0
        for i in range(length):
            val = buf[i]
            # Nếu là bytes hoặc bytearray, lấy giá trị đầu tiên
            if isinstance(val, (bytes, bytearray)):
                val = val[0]
            result ^= val  # XOR checksum
        return bytes([result])

    def send(self, byte_send):
        self.lora_ser.write(byte_send)

    def register_handle_lora_msg(self, handle_lora_msg):
        self.handle_lora_msg = handle_lora_msg

    def clear_buf(self):
        self.lora_msg.clear()
    
    def add_to_buf(self, byte_recv):
        self.lora_msg.append(byte_recv)
        
    def read(self):
        """
            This method use to read data from lora_ser and update data to lora_msg
        """
        if self.lora_ser == None:
            return
        
        try:
            byte_recv = self.lora_ser.read(1)
            if self.fsm_read_status == self.FSM_READ_INIT:
                if byte_recv == b'!':
                    self.clear_buf()
                    self.add_to_buf(byte_recv)
                    # change fsm status
                    self.fsm_read_status = self.FSM_READ_BEGIN
            elif self.fsm_read_status == self.FSM_READ_BEGIN:
                if byte_recv == b'!':
                    self.clear_buf()
                    self.add_to_buf(byte_recv)
                elif byte_recv == b'#':
                    self.add_to_buf(byte_recv)
                    # change fsm status
                    self.fsm_read_status = self.FSM_READ_END
                else:
                    self.add_to_buf(byte_recv)
            elif self.fsm_read_status == self.FSM_READ_END:
                # check sum
                expect_checksum = self.calculate_checksum(self.lora_msg, len(self.lora_msg))
                if byte_recv[0] == expect_checksum[0]:
                    print("check sum success")
                    if self.handle_lora_msg != None:
                        # Send ack
                        # Chuyển tất cả phần tử thành int
                        byte_send_list = [b[0] if isinstance(b, (bytes, bytearray)) else b for b in self.lora_msg[0:3]]
                        byte_send_list.append(ord('#'))  # hoặc b'#'[0]
                        byte_send_list.append(self.calculate_checksum(byte_send_list, 4)[0])

                        # Chuyển sang bytes
                        byte_send = bytes(byte_send_list)
                        print(byte_send)

                        self.send(byte_send)

                        self.handle_lora_msg(self.lora_msg[1:-1])
                        
                    self.fsm_read_status = self.FSM_READ_INIT
                else:
                    print("check sum failed")
                    print("recv checksum:", byte_recv)
                    print("expected checksum:", expect_checksum)
                    self.add_to_buf(byte_recv)

                    self.fsm_read_status = self.FSM_READ_BEGIN


        except SerialException:
            print(f"Disconnect from {self.lora_ser}")
            self.lora_ser = None

LORA_COM_PORT = sys.argv[1]
# LORA_COM_PORT = "/dev/pts/4"
lora = Lora(LORA_COM_PORT)

if __name__ == "__main__":
    while True:
        lora.read()