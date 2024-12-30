import os
import struct
import serial
import time

# Constants
HEADER_CHAR = 0xAA
OTA_PACKET_SIZE = 512
PACKET_HEADER = 0x00
PACKET_NORMAL = 0x02
PACKET_END = 0x03
COMMUNICATION_SPEED = 921600

SERIAL_PORT = "/dev/tty.wchusbserial830"  # Update to your serial port
BAUD_RATE = 115200
ACK_TIMEOUT = 2  # Timeout in seconds


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
        print(f"Packet ID: {self.packet_id}, CRC: {hex(self.crc)}")

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
        print(f"Packet ID: {instance.packet_id}, CRC: {hex(instance.crc)}")

        return instance

    def pack(self):
        """
        Pack the DataPacket into bytes for transmission.
        """
        return struct.pack("<BBHH", self.header, self.type, self.packet_id, self.payload_size) + self.payload + struct.pack(">H", self.crc)


def wait_for_ack(serial_port):
    start_time = time.time()
    received_data = b""  # Buffer to store received data
    print("[BELOW FROM ESP32]")
    while True:
        if time.time() - start_time > ACK_TIMEOUT:
            print("[END FROM ESP32]")
            print("ACK timeout.")
            if received_data:
                print("Data received before timeout:")
                print(received_data.decode('ascii', errors='replace'))
            return False

        if serial_port.in_waiting > 0:
            char = serial_port.read(1)  # Read one character
            received_data += char  # Append to buffer

            # Print the character to the screen
            print(char.decode('ascii', errors='replace'), end='', flush=True)

            if char == b'\xAA':  # Success ACK
                print("[END FROM ESP32]")
                print("\nReceived success ACK.")
                return True
            elif char == b'\xFF':  # Failure ACK
                print("[END FROM ESP32]")
                print("\nReceived failure ACK.")
                return False


def send_packet(serial_port, packet):
    packed_data = packet.pack()
    serial_port.write(packed_data)
    serial_port.flush()
    return wait_for_ack(serial_port)


def restart_serial_port(port, baud_rate, timeout=ACK_TIMEOUT):
    """
    Restart the serial port with the given baud rate.

    :param port: The serial port name.
    :param baud_rate: The new baud rate to set.
    :param timeout: Timeout for the serial connection.
    :return: A new serial port instance.
    """
    print(f"[PYTHON] Restarting serial port {port} with baud rate {baud_rate}...")
    time.sleep(1)  # Short delay to allow the port to reset
    return serial.Serial(port, baud_rate, timeout=timeout)


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
            header_payload = header_packet.pack()
            print(header_payload.hex())
            header_data_packet = DataPacket.from_parameters(HEADER_CHAR, PACKET_HEADER, 0, header_payload, len(header_payload))
            print("Sending header packet...")
            if not send_packet(ser, header_data_packet):
                print("Failed to send header packet.")
                exit(1)
            print("Header packet sent successfully.")

            # Change baud rate and restart serial port
            ser.close()  # Close the current serial port
            ser = restart_serial_port(SERIAL_PORT, COMMUNICATION_SPEED)  # Restart with new baud rate
            time.sleep(0.1)

            # Send firmware data packets
            with open(FIRMWARE_PATH, "rb") as firmware:
                packet_id = 1
                while True:
                    chunk = firmware.read(OTA_PACKET_SIZE)
                    if not chunk:
                        break
                    is_final_packet = firmware.tell() == total_firmware_size
                    data_packet = DataPacket(packet_id, chunk, is_final_packet)
                    print(f"Sending packet ID: {data_packet.packet_id}")
                    if not send_packet(ser, data_packet):
                        print(f"Failed to send packet ID: {data_packet.packet_id}")
                        exit(1)
                    print(f"Packet ID {data_packet.packet_id} sent successfully.")
                    packet_id += 1
                    time.sleep(0.1)
            print("Firmware transmission completed successfully.")

    except FileNotFoundError:
        print(f"Error: Firmware file '{FIRMWARE_PATH}' not found.")
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")