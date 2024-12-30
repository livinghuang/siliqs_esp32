#include "simulate_raw_payload.h"
#include "simulate_packet.h"

void payload_creator(struct payload_data &payload)
{
  size_t firmwareSize = sizeof(firmwareData);
  static bool isFinalPiece = false;
  static size_t offset = 0;
  char *firmwareDataInPiece;
  size_t piece_size = 0;
  const size_t CHUNK_SIZE = 512;

  isFinalPiece = (offset + CHUNK_SIZE >= firmwareSize);

  if (isFinalPiece)
  {
    piece_size = firmwareSize % CHUNK_SIZE;
  }
  else
  {
    piece_size = CHUNK_SIZE;
  }

  firmwareDataInPiece = (char *)firmwareData + offset;

  offset += piece_size;

  // 输出调试信息
  Serial.printf("current piece size: %ld\n", piece_size);
  Serial.printf("offset: %ld\n", offset);

  if ((isFinalPiece) && (offset == firmwareSize))
  {
    Serial.println("complete Raw Payload end");
  }
  payload.length = piece_size;
  payload.isFinalPacket = isFinalPiece;
  memcpy(payload.data, firmwareDataInPiece, piece_size);
}

struct DataPacket global_data_packet;
void simulate_raw_init()
{
  Serial.println("Simulate Raw Payload start");
  create_packet_header(global_data_packet);
  // print packet header
  Serial.printf("Packet ID: %d\n", global_data_packet.packet_id);
  Serial.printf("Packet Type: %d\n", global_data_packet.type);
  Serial.printf("Packet Payload Size: %d\n", global_data_packet.payload_size);
  Serial.printf("Packet CRC: %d\n", global_data_packet.crc);
  Serial.flush();
  if (validatePacket(global_data_packet))
  {
    Serial.println("validatePacket success");
  }
}

void simulate_raw_loop()
{
  struct payload_data payload;
  payload_creator(payload);

  create_packet_normal(global_data_packet, payload);

  Serial.printf("Packet ID: %d\n", global_data_packet.packet_id);
  if (global_data_packet.type == PACKET_NORMAL)
  {
    Serial.printf("Packet Type: PACKET NORMAL\n");
  }
  else if (global_data_packet.type == PACKET_END)
  {
    Serial.printf("Packet Type: PACKET END\n");
  }
  Serial.printf("Packet Payload Size: %d\n", global_data_packet.payload_size);
  Serial.printf("Packet CRC: %d\n", global_data_packet.crc);
  Serial.flush();

  if (validatePacket(global_data_packet))
  {
    Serial.println("validatePacket success");
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
      Serial.printf("write data block success,  written: %d\n", written);
      if (global_data_packet.type == PACKET_END)
      {
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
  else
  {
    while (1)
    {
      Serial.println("validatePacket failed");
      delay(1000);
    }
  }
}