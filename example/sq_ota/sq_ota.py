import serial
import struct
import time
import crcmod


# Constants for the DataPacket format
HEADER_HIGH = 0xAA
HEADER_LOW = 0x55
PACKET_TYPE_OTA = 0x02
CHUNK_SIZE = 512  # Maximum data payload size

# ACK Constants
ACK_SUCCESS = 0x06  # ESP32 sends 0x06 for success
ACK_FAILURE = 0x15  # ESP32 sends 0x15 for failure
ACK_TIMEOUT = 5  # Timeout in seconds to wait for ACK

# Configure the serial port
SERIAL_PORT = "/dev/tty.wchusbserial830"  # Update this to your ESP32 serial port
BAUD_RATE = 115200

# Path to the firmware file
FIRMWARE_PATH = "example/sq_ota/sq_empty.ino.bin"  # Update this to your firmware file path


def calculate_crc(data):
    """
    Calculate CRC16 checksum for the given data.

    :param data: Data to calculate checksum for
    :return: CRC16 value
    """
    crc16 = crcmod.mkCrcFun(0x11021, initCrc=0xFFFF, xorOut=0x0000)
    return crc16(data)


def create_packet(data, packet_type=PACKET_TYPE_OTA):
    """
    Build a data packet with the given data and packet type.

    :param data: Payload data
    :param packet_type: Packet type
    :return: Structured packet ready for transmission
    """
    length = len(data)
    header = struct.pack("BB", HEADER_HIGH, HEADER_LOW)
    type_field = struct.pack("B", packet_type)
    length_field = struct.pack(">H", length)  # Unsigned short (big endian)
    crc_data = header + type_field + length_field + data
    crc = calculate_crc(crc_data)
    crc_field = struct.pack(">H", crc)  # Unsigned short (big endian)
    return header + type_field + length_field + data + crc_field


def wait_for_ack(serial_port):
    """
    Wait for an acknowledgment from the ESP32.

    :param serial_port: Serial port instance
    :return: ACK byte or None if timeout
    """
    start_time = time.time()
    while time.time() - start_time < ACK_TIMEOUT:
        if serial_port.in_waiting > 0:
            ack = serial_port.read(1)
            return ord(ack)  # Return the received byte as an integer
    return None  # Timeout


def send_firmware(serial_port, firmware_path):
    """
    Send the firmware to the ESP32 in chunks and handle ACKs.

    :param serial_port: Serial port instance
    :param firmware_path: Path to the firmware binary file
    """
    try:
        with open(firmware_path, "rb") as firmware:
            total_size = 0
            print(f"Starting firmware upload from {firmware_path}...")
            while True:
                chunk = firmware.read(CHUNK_SIZE)
                if not chunk:
                    break  # End of file

                # Create a data packet
                packet = create_packet(chunk)

                # Send the packet
                serial_port.write(packet)
                total_size += len(chunk)

                # Wait for acknowledgment
                ack = wait_for_ack(serial_port)
                if ack == ACK_SUCCESS:
                    print(f"ACK received for chunk (Total: {total_size} bytes)")
                elif ack == ACK_FAILURE:
                    print(f"ACK failure for chunk (Total: {total_size} bytes). Retrying...")
                    total_size -= len(chunk)
                    firmware.seek(total_size)  # Rewind the file pointer
                else:
                    print(f"ACK timeout for chunk (Total: {total_size} bytes). Retrying...")
                    total_size -= len(chunk)
                    firmware.seek(total_size)  # Rewind the file pointer

                time.sleep(0.1)  # Small delay to avoid overwhelming the serial buffer

            print("Firmware upload completed successfully.")

    except FileNotFoundError:
        print(f"Error: Firmware file '{firmware_path}' not found.")
    except Exception as e:
        print(f"Error during firmware upload: {e}")


def main():
    """
    Main function to open the serial port and start the firmware upload process.
    """
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
            send_firmware(ser, FIRMWARE_PATH)
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


if __name__ == "__main__":
    main()
