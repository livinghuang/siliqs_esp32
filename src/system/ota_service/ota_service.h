#pragma once
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

bool validatePacket(const DataPacket &packet);
String server_param_receive(const String &param);