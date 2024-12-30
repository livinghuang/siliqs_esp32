#include "siliqs_esp32.h"

#define ACK_SUCCESS 0xAA
#define ACK_FAILURE 0xFF

#define HEADER_CHAR 0xAA
#define OTA_PACKET_SIZE 512
#define PACKET_HEADER 0x00
#define PACKET_NORMAL 0x02
#define PACKET_END 0x03
#define INVALID_PACKET 0xFF
#define COMMUNICATION_SPEED 115200

#define UNKNOWN_UPLOAD_FIRMWARE_SIZE 0xFFFFFFFF

#ifdef USE_NIMBLE
// 创建 BLEATCommandService 实例
BLEATCommandService BLEatService;
#endif
// 创建 UARTATCommandService 实例，使用 Serial 作为通信接口
UARTATCommandService UARTatService;

/*
  Define OTA packet structure
  Header = HEADER_CHAR as 0xAA
  Type = PACKET_HEADER  as PACKET_NORMAL, PACKET_END
  Packet ID = Packet ID
  Payload size = Size of the payload
  Payload = Payload data (adjust size based on needs)
  CRC = crc for error detection
*/

struct DataPacket
{
  uint8_t header;                   // Header
  uint8_t type;                     // packet type, 0x00 :normal packet, 0x01 : packet end
  uint16_t packet_id;               // Unique ID for the packet
  uint16_t payload_size;            // Size of the payload
  uint8_t payload[OTA_PACKET_SIZE]; // Payload data (adjust size based on needs)
  uint16_t crc;                     // crc for error detection
};

/*
Define Header Packet Payload structure, the header packet payload will show communication status for handshake
*/
struct DataHeaderPacket
{
  size_t total_firmware_size;
  size_t each_packet_size;
  size_t total_packets;
  size_t communication_speed;
};

struct payload_data
{
  uint8_t data[512];
  uint16_t length;
  bool isFinalPacket;
};

const int ledPin = 2; // 替换为实际的引脚号
size_t global_information_total_firmware_size;
size_t global_information_each_packet_size;
size_t global_information_total_packets;
size_t global_information_communication_speed;

bool validatePacket(const DataPacket &packet);

void setup()
{
  siliqs_esp32_setup(SQ_DEBUG);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  const char *partitionLabel = NULL;
  if (sqUpdate.begin())
  {
    Serial.println("开始 OTA 更新...");
  }
  else
  {
    Serial.printf("启动 OTA 更新失败: \n");
  }
}

void sendAck(uint8_t ackType)
{
  delay(100);
  Serial.write(ackType);
}

bool processPacket(DataPacket &packet)
{
  static size_t receivedFirmwareSize = 0;
  static size_t expectedFirmwareSize = 0;
  bool result = false;
  if (packet.header != HEADER_CHAR)
  {
    Serial.println("Invalid header received.");
    result = false;
    return false;
  }

  if (!validatePacket(packet))
  {
    Serial.println("CRC validation failed.");
    result = false;
    return false;
  }

  if (packet.type == PACKET_HEADER)
  {
    DataHeaderPacket *header = (DataHeaderPacket *)packet.payload;
    expectedFirmwareSize = header->total_firmware_size;
    result = true;
  }
  else if (packet.type == PACKET_NORMAL || packet.type == PACKET_END)
  {
    // Write the payload to flash or buffer (implementation based on your requirements)
    receivedFirmwareSize += packet.payload_size;

    Serial.printf("Data Packet Received: ID: %d, Size: %d, Total Received: %d\n",
                  packet.packet_id, packet.payload_size, receivedFirmwareSize);

    if (packet.type == PACKET_END)
    {
      Serial.println("Final Packet Received. Verifying...");
      if (receivedFirmwareSize == expectedFirmwareSize)
      {
        Serial.println("Firmware received successfully.");
        result = true;
      }
      else
      {
        Serial.println("Firmware size mismatch!");
        result = false;
      }
    }
    else
    {
      result = true;
    }
  }
  else
  {
    Serial.println("Invalid packet type received.");
    result = false;
  }
  return result;
}
void loop()
{
  static uint8_t buffer[sizeof(DataPacket)];
  static size_t bytesRead = 0;

  // Read incoming data from the serial port
  while (Serial.available())
  {
    buffer[bytesRead++] = Serial.read();

    // Check if a full packet has been received
    if (bytesRead == sizeof(DataPacket))
    {
      DataPacket *packet = (DataPacket *)buffer;
      if (processPacket(*packet))
      {
        if (packet->type == PACKET_HEADER)
        {
          global_information_total_firmware_size = ((DataHeaderPacket *)packet->payload)->total_firmware_size;
          global_information_each_packet_size = ((DataHeaderPacket *)packet->payload)->each_packet_size;
          global_information_total_packets = ((DataHeaderPacket *)packet->payload)->total_packets;
          global_information_communication_speed = ((DataHeaderPacket *)packet->payload)->communication_speed;
          sqUpdate.setUpdateSize(global_information_total_firmware_size);
          Serial.printf("Header Packet Received:\n");
          Serial.printf("  Total Firmware Size: %ld\n", global_information_total_firmware_size);
          Serial.printf("  Each Packet Size: %ld\n", global_information_each_packet_size);
          Serial.printf("  Total Packets: %ld\n", global_information_total_packets);
          Serial.printf("  Communication Speed: %ld\n", global_information_communication_speed);
        }
        else
        {
          DataPacket global_data_packet;
          memcpy(&global_data_packet, packet, sizeof(DataPacket));

          if (global_data_packet.type == PACKET_NORMAL)
          {
            Serial.printf("Normal Packet Received: ID: %d, Size: %d\n", global_data_packet.packet_id, global_data_packet.payload_size);
          }
          else if (global_data_packet.type == PACKET_END)
          {
            Serial.printf("Final Packet Received: ID: %d, Size: %d\n", global_data_packet.packet_id, global_data_packet.payload_size);
          }

          size_t written = sqUpdate.write512(global_data_packet.payload, global_data_packet.payload_size, global_data_packet.type == PACKET_END);
          if (written != global_data_packet.payload_size)
          {
            while (1)
            {
              Serial.printf("write data block failed, written: %d, expected: %d\n", written, global_data_packet.payload_size);
              delay(1000);
            }
          }
          else
          {
            if (global_data_packet.type == PACKET_END)
            {
              sendAck(ACK_SUCCESS);
              if (sqUpdate.end())
              {
                Serial.println("OTA 更新成功");
              }
              else
              {
                Serial.printf("OTA 更新失败: %s\n", sqUpdate.errorString());
              }
              while (1)
              {
                Serial.println("Simulate Raw Payload end");
                delay(1000);
              }
            }
          }
        }
        sendAck(ACK_SUCCESS);
        if (packet->type == PACKET_HEADER)
        {
          delay(100);
          Serial.end();
          delay(100);
          Serial.begin(global_information_communication_speed);
          delay(100);
        }
      }
      else
      {
        sendAck(ACK_FAILURE);
      }
      // Reset for the next packet
      bytesRead = 0;
    }
  }
}
uint16_t swapBytes(uint16_t value)
{
  // Swap high and low bytes
  return (value >> 8) | (value << 8);
}

bool validatePacket(const DataPacket &packet)
{
  if (packet.header != HEADER_CHAR)
  {
    Serial.println("Invalid packet header.");
    return false;
  }

  // Calculate the CRC for the packet (excluding the CRC field itself)
  uint16_t calculated_crc = calculateCRC(reinterpret_cast<const uint8_t *>(&packet), offsetof(DataPacket, crc));

  // Swap the high and low bytes of the received CRC
  uint16_t swapped_crc = swapBytes(packet.crc);

  // Compare the calculated CRC with the swapped CRC
  if ((calculated_crc == swapped_crc) || (calculated_crc == packet.crc))
  {
    return true;
  }
  // Print the CRC values for debugging
  Serial.printf("CRC mismatch! Calculated: 0x%04X, Received: 0x%04X (Swapped: 0x%04X)\n",
                calculated_crc, packet.crc, swapped_crc);
  return false;
}