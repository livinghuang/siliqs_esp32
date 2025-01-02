import os
import struct
import serial
import time
import base64

# Constants
HEADER_CHAR = 0xAA
OTA_PACKET_SIZE = 256
PACKET_HEADER = 0x00
PACKET_NORMAL = 0x02
PACKET_END = 0x03
COMMUNICATION_SPEED = 115200

SERIAL_PORT = "/dev/tty.wchusbserial830"  # Update to your serial port
BAUD_RATE = 115200
ACK_TIMEOUT = 2  # Timeout in seconds

# debug = True
debug = False

def calculate_crc(data):
    """
    Calculate the CRC-16/Modbus checksum for the given data.
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc


# DataHeaderPacket
class DataHeaderPacket:
    def __init__(self, total_firmware_size, each_packet_size, total_packets, communication_speed):
        self.total_firmware_size = total_firmware_size
        self.each_packet_size = each_packet_size
        self.total_packets = total_packets
        self.communication_speed = communication_speed

    def pack(self):
        return struct.pack("<IIII", self.total_firmware_size, self.each_packet_size, self.total_packets, self.communication_speed)



# DataPacket
class DataPacket:
    def __init__(self, packet_id, payload, is_final_packet):
        """
        Default constructor for creating a DataPacket with automatic CRC calculation.
        """
        self.header = HEADER_CHAR
        self.type = PACKET_END if is_final_packet else PACKET_NORMAL
        self.packet_id = packet_id
        self.payload_size = len(payload)
        self.payload = payload.ljust(OTA_PACKET_SIZE, b'\xFF')  # Pad with 0xFF
        crc_data = struct.pack("<BBHH", self.header, self.type, self.packet_id, self.payload_size) + self.payload
        self.crc = calculate_crc(crc_data)

        # Print CRC value
        # print(f"Packet ID: {self.packet_id}, CRC: {hex(self.crc)}")

    @classmethod
    def from_parameters(cls, header, packet_type, packet_id, payload, payload_size):
        """
        Alternative constructor to initialize all parameters directly.
        """
        instance = cls.__new__(cls)  # Create an instance without calling __init__
        instance.header = header
        instance.type = packet_type
        instance.packet_id = packet_id
        instance.payload_size = payload_size
        instance.payload = payload.ljust(OTA_PACKET_SIZE, b'\xFF')  # Pad with 0xFF
        crc_data = struct.pack("<BBHH", instance.header, instance.type, instance.packet_id, instance.payload_size) + instance.payload
        instance.crc = calculate_crc(crc_data)

        # Print CRC value
        # print(f"Packet ID: {instance.packet_id}, CRC: {hex(instance.crc)}")

        return instance

    def pack(self):
        """
        Pack the DataPacket into bytes for transmission.
        """
        return struct.pack("<BBHH", self.header, self.type, self.packet_id, self.payload_size) + self.payload + struct.pack(">H", self.crc)
    def packBase64(self):
        """
        Pack the DataPacket into bytes and encode it as a Base64 string.
        """
        packed_data = self.pack()  # Get the packed data as bytes
        base64_encoded = base64.b64encode(packed_data).decode('utf-8')  # Encode the packed data as base64 and decode it to UTF-8 string
        return base64_encoded

def wait_for_ack(serial_port):
    start_time = time.time()
    received_data = ""  # Buffer to store received data as a string
    if debug:
        print("[BELOW FROM ESP32]")

    while True:
        if time.time() - start_time > ACK_TIMEOUT:
            if debug:
                print("[END FROM ESP32]")
            print("ACK timeout.")
            if received_data:
                print("Data received before timeout:")
                print(received_data)
            return False

        if serial_port.in_waiting > 0:
            # Read available bytes and decode as ASCII
            data_chunk = serial_port.read(serial_port.in_waiting).decode('ascii', errors='replace')
            received_data += data_chunk  # Append to the string buffer

            # Print the chunk to the screen if in debug mode
            if debug:
                print(data_chunk, end='', flush=True)

            # Check if we have received the full success or error acknowledgment
            if "SQ OK" in received_data:
                if debug:
                    print("\n[END FROM ESP32]")
                # print("\nReceived success ACK (SQ OK).")
                return True
            elif "SQ ERROR" in received_data:
                if debug:
                    print("\n[END FROM ESP32]")
                print("\nReceived failure ACK (SQ ERROR).")
                return False



def send_packet(serial_port, packet):
    serial_port.write(packet)
    serial_port.flush()
    return wait_for_ack(serial_port)



def send_atota_command(header,binary_data,baud_rate=BAUD_RATE):
    """
    Send the ATOTA=B command to the serial port to start the OTA process.
    """
    ser.reset_input_buffer()
    command='ATOTA='+header+binary_data
    atota_command = command.encode()
    ser.write(atota_command)
    result = wait_for_ack(ser)
    return result


# Main program
if __name__ == "__main__":
    FIRMWARE_PATH = "/Users/living/Documents/Arduino/libraries/siliqs_esp32/example/sq_ota/Blink.ino.bin"
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=ACK_TIMEOUT) as ser:
            total_firmware_size = os.path.getsize(FIRMWARE_PATH)
            total_packets = total_firmware_size // OTA_PACKET_SIZE
            if total_firmware_size % OTA_PACKET_SIZE != 0:
                total_packets += 1

            print(f"[PYTHON] Total firmware size: {total_firmware_size} bytes")
            print(f"[PYTHON] Total packets: {total_packets}")

            # Create header packet
            header_packet = DataHeaderPacket(
                total_firmware_size=total_firmware_size,
                each_packet_size=OTA_PACKET_SIZE,
                total_packets=total_packets,
                communication_speed=COMMUNICATION_SPEED,
            )

            # Create header packet
            header_payload = header_packet.pack()  # Pack the header into bytes
            header_data_packet = DataPacket.from_parameters(HEADER_CHAR, PACKET_HEADER, 0, header_payload, len(header_payload))
            # Encode the header_payload as Base64 and convert it to a string
            encoded_header_data_packet = header_data_packet.packBase64()
            # Construct the full ATOTA command string
            command_with_end_char = f"{encoded_header_data_packet}\r\n"  # Adding carriage return and line feed at the end
            # Send the full command as byte data
            if not send_atota_command('B',command_with_end_char):
                print("[PYTHON] Failed to send header packet.")
                ser.close()
                exit(1)
            ser.close()

            # Send firmware data packets
            with open(FIRMWARE_PATH, "rb") as firmware:
                packet_id = 0
                while True:
                    chunk = firmware.read(OTA_PACKET_SIZE)
                    if not chunk:
                        break
                    is_final_packet = firmware.tell() == total_firmware_size
                    data_packet = DataPacket(packet_id, chunk, is_final_packet)
                    # Encode the header_payload as Base64 and convert it to a string
                    encoded_data_packet = data_packet.packBase64()
                    # Construct the full ATOTA command string
                    command_with_end_char = f"{encoded_data_packet}\r\n"  # Adding carriage return and line feed at the end
                    if is_final_packet:
                    # Send the full command as byte data
                        ser.open()
                        if not send_atota_command('E',command_with_end_char):
                            print("[PYTHON] Failed to send header packet.")
                            ser.close()
                            exit(1)
                        ser.close()
                        break
                    else:
                    # Send the full command as byte data   
                        ser.open()
                        if not send_atota_command('N',command_with_end_char):
                            print("Failed to send header packet.")
                            ser.close()
                            exit(1)
                        ser.close()
                    percentage = (packet_id / total_packets) * 100
                    print(f"[STATUS]({packet_id}/{total_packets}, {percentage:.1f}%)")
                    packet_id += 1
            print("Firmware transmission completed successfully.")
            time.sleep(2)
            ser.close()
            exit(1)

    except FileNotFoundError:
        print(f"Error: Firmware file '{FIRMWARE_PATH}' not found.")
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")