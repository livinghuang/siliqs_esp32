/**
 * @file ota_debug_serial.ino
 * @brief OTA update using structured data received from the debug serial port
 *
 * This project demonstrates Over-The-Air (OTA) update functionality using the debug port.
 * Firmware data is transmitted in a structured data format via the serial port.
 * The `Update.h` library is utilized for robust handling of firmware updates.
 *
 * ************
 * NOTE: Under development
 * ************
 */

#include <Arduino.h>
#include <Update.h>

// Define structured data format for firmware transmission
#define HEADER_HIGH 0xAA
#define HEADER_LOW 0x55

struct DataPacket
{
  uint8_t header[2]; // Packet header (0xAA, 0x55)
  uint8_t type;      // Packet type (0x02 for OTA data)
  uint16_t length;   // Length of the data payload
  uint8_t data[512]; // Data payload (maximum 512 bytes)
  uint16_t crc;      // CRC16 for error checking
};

// Function prototypes
bool validatePacket(const DataPacket &packet);
uint16_t calculateCRC(const uint8_t *data, size_t length);
void processPacket(const DataPacket &packet);

void setup()
{
  Serial.begin(115200); // Initialize debug serial port
  Serial.println("Waiting for OTA packets...");

  // Begin update setup (size will be validated in packets)
  if (!Update.begin(UPDATE_SIZE_UNKNOWN))
  {
    Serial.println("Failed to initialize update process!");
  }
}

void loop()
{
  static uint8_t buffer[sizeof(DataPacket)]; // Buffer to store incoming data
  static size_t bufferIndex = 0;

  // Read from Serial and fill the buffer
  while (Serial.available())
  {
    buffer[bufferIndex++] = Serial.read();

    // If the buffer contains a complete packet
    if (bufferIndex == sizeof(DataPacket))
    {
      DataPacket packet;
      memcpy(&packet, buffer, sizeof(DataPacket));

      // Validate the packet
      if (validatePacket(packet))
      {
        processPacket(packet); // Process the valid packet
      }
      else
      {
        Serial.println("Invalid packet received!");
      }

      bufferIndex = 0; // Reset buffer for the next packet
    }
  }

  // If the update is finished
  if (Update.isFinished())
  {
    if (Update.end())
    {
      Serial.println("OTA Update complete! Restarting...");
      ESP.restart(); // Restart to apply the update
    }
    else
    {
      Serial.printf("OTA Update failed: %s\n", Update.errorString());
    }
  }
}

/**
 * @brief Validate the received packet
 * @param packet The received data packet
 * @return true if the packet is valid, false otherwise
 */
bool validatePacket(const DataPacket &packet)
{
  // Check the packet header
  if (packet.header[0] != HEADER_HIGH || packet.header[1] != HEADER_LOW)
  {
    return false; // Invalid header
  }

  // Calculate CRC for the packet
  uint16_t calculatedCRC = calculateCRC(reinterpret_cast<const uint8_t *>(&packet),
                                        sizeof(packet) - sizeof(packet.crc));
  return (calculatedCRC == packet.crc);
}

/**
 * @brief Calculate CRC16 for error checking
 * @param data Pointer to the data
 * @param length Length of the data
 * @return Calculated CRC16 value
 */
uint16_t calculateCRC(const uint8_t *data, size_t length)
{
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++)
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc;
}
void processPacket(const DataPacket &packet)
{
  char ACK_SUCCESS = 0xAA;
  char ACK_FAILURE = 0xFF;
  // Check the packet type
  if (packet.type == 0x02)
  {                                                                       // OTA data type
    size_t written = Update.write((uint8_t *)packet.data, packet.length); // Cast to non-const
    if (written == packet.length)
    {
      Serial.write(ACK_SUCCESS); // Send ACK for success
    }
    else
    {
      Serial.write(ACK_FAILURE); // Send ACK for failure
    }

    // If the update is finished
    if (Update.isFinished())
    {
      if (Update.end())
      {
        Serial.println("OTA Update complete! Restarting...");
        ESP.restart(); // Restart to apply the update
      }
      else
      {
        Serial.printf("OTA Update failed: %s\n", Update.errorString());
      }
    }
  }
  else
  {
    Serial.println("Unsupported packet type received.");
  }
}
