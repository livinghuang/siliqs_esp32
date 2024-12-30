#include "simulate_packet.h"

void create_packet_header(struct DataPacket &packet)
{
  packet.header = HEADER_CHAR;
  packet.type = PACKET_HEADER;
  packet.packet_id = 0;
  struct DataHeaderPacket header_packet;
  header_packet.total_firmware_size = sizeof(firmwareData);
  header_packet.each_packet_size = OTA_PACKET_SIZE;
  header_packet.total_packets = sizeof(firmwareData) / OTA_PACKET_SIZE;
  if (sizeof(firmwareData) % OTA_PACKET_SIZE != 0)
  {
    header_packet.total_packets++;
  }
  header_packet.communication_speed = COMMUNICATION_SPEED;
  memcpy(packet.payload, (uint8_t *)&header_packet, sizeof(header_packet));
  packet.payload_size = sizeof(header_packet);
  packet.crc = calculateCRC((uint8_t *)&packet, sizeof(packet) - sizeof(packet.crc));
}

void create_packet_normal(struct DataPacket &packet, struct payload_data piece)
{
  static uint16_t packet_id = 1;
  packet.header = HEADER_CHAR;
  if (!piece.isFinalPacket)
  {
    packet.type = PACKET_NORMAL;
  }
  else
  {
    packet.type = PACKET_END;
  }
  packet.packet_id = packet_id;
  packet.payload_size = piece.length;
  memcpy(packet.payload, piece.data, piece.length);
  packet.crc = calculateCRC((uint8_t *)&packet, sizeof(packet) - sizeof(packet.crc));
  packet_id++;
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
