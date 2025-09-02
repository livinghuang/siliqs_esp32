#include "ota_service.h"

#define OTA_PARAM_START_CHAR 'S'
#define OTA_PARAM_END_CHAR 'E'
#define OTA_PARAM_NORMAL_PACK 'N'
#define OTA_PARAM_BEGIN_PACK 'B'

size_t global_information_total_firmware_size;
size_t global_information_each_packet_size;
size_t global_information_total_packets;
size_t global_information_communication_speed;

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
  uint16_t swapped_crc;
  swap_high_low_bytes(&swapped_crc);
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

bool decode_and_validate_packet(const String &payload, uint8_t *decodedData, size_t &decodedLength)
{
  decodedLength = Base64::decodedLength(payload.c_str());
  Serial.printf("base64 decoded length: %d\n", decodedLength);
  Base64::decode((char *)decodedData, payload.c_str(), payload.length());
  if (decodedLength < sizeof(DataPacket))
  {
    Serial.println("invalid payload length");
    Serial.printf("DataPacket size :%d", sizeof(DataPacket));
    return false;
  }
  return true;
}

bool ota_write_data_execute_normal_packet(DataPacket *packet)
{
  Serial.printf("Normal Packet Received: ID: %d, Size: %d\n", packet->packet_id, packet->payload_size);
  size_t written = sqUpdate.writebychunksize(packet->payload, packet->payload_size, OTA_PACKET_SIZE, packet->type == PACKET_END);
  if (written != packet->payload_size)
  {
    Serial.printf("write data block failed, written: %d, expected: %d\n", written, packet->payload_size);
    return false;
  }
  return true;
}

bool begin_pack(const String &payload)
{
  uint8_t decodedData[Base64::decodedLength(payload.c_str())];
  size_t decodedLength = 0;

  Serial.println("Begin OTA updating...");
  if (!decode_and_validate_packet(payload, decodedData, decodedLength))
  {
    return false;
  }

  DataPacket *packet = (DataPacket *)decodedData;

  global_information_total_firmware_size = ((DataHeaderPacket *)packet->payload)->total_firmware_size;
  global_information_each_packet_size = ((DataHeaderPacket *)packet->payload)->each_packet_size;
  global_information_total_packets = ((DataHeaderPacket *)packet->payload)->total_packets;
  global_information_communication_speed = ((DataHeaderPacket *)packet->payload)->communication_speed;

  sqUpdate.setUpdateSize(global_information_total_firmware_size);

  Serial.printf("total_firmware_size: %d\n", global_information_total_firmware_size);
  Serial.printf("packet_size: %d\n", global_information_each_packet_size);
  Serial.printf("total_packets: %d\n", global_information_total_packets);
  Serial.printf("communication_speed: %d\n", global_information_communication_speed);
  Serial.flush();

  if (sqUpdate.begin())
  {
    Serial.println("start OTA updating...");
    return true;
  }
  else
  {
    Serial.printf("begin OTA updating failed: \n");
    return false;
  }
}

bool normal_pack(const String &payload)
{
  uint8_t decodedData[Base64::decodedLength(payload.c_str())];
  size_t decodedLength = 0;

  if (!decode_and_validate_packet(payload, decodedData, decodedLength))
  {
    Serial.println("invalid packet");
    return false;
  }
  return ota_write_data_execute_normal_packet((DataPacket *)decodedData);
}

String server_param_receive(const String &param)
{
  char param_char = param.charAt(0);
  String payload = param.substring(1, param.length());
  // Serial.printf("Payload: %s\n", payload.c_str());

  switch (param_char)
  {
  case OTA_PARAM_START_CHAR:
    return "START SQ OK";
  case OTA_PARAM_BEGIN_PACK:
    if (begin_pack(payload))
    {
      return "BEGIN SQ OK";
    }
    else
    {
      return "BEGIN SQ ERROR";
    }
    break;

  case OTA_PARAM_NORMAL_PACK:
    if (normal_pack(payload))
    {
      return "NORMAL SQ OK";
    }
    else
    {
      return "NORMAL SQ ERROR";
    }
    break;

  case OTA_PARAM_END_CHAR:
    if (normal_pack(payload))
    {
      if (sqUpdate.end())
      {
        Serial.println("OTA update completed successfully!");
        Serial.println("New partition activated. Restarting...");
        Serial.println("END SQ OK");
        delay(1000);
        esp_restart(); // 重启设备以加载新固件
        return "END SQ OK";
      }
      else
      {
        Serial.printf("OTA update failed: %s\n", sqUpdate.errorString());
        return "END SQ ERROR";
      }
    }
    else
    {
      Serial.printf("OTA PARAM normal_pack(payload) ERROR\n");
      return "END SQ ERROR";
    }
    break;
  }
  return "SQ ERROR";
}