#include "bsp.h"
#ifdef USE_MODBUS
#include "modbus_communication.h"
#include <string.h> // 用于 memcpy 函数

// 构造函数
ModbusCommunication::ModbusCommunication(HardwareSerial &serial, int baudRate, int RO, int DI, int directionPin, int powerPin)
    : RS485Communication(serial, baudRate, RO, DI, directionPin, powerPin) {}

// 发送 Modbus 数据帧
void ModbusCommunication::send_modbus(const modbus_data_t *modbusData)
{
  // 打包 Modbus 数据帧为字节数组
  uint8_t buffer[MODBUS_MAX_DATA_LENGTH + 4]; // 数据+地址(1)+功能码(1)+CRC(2)
  uint16_t index = 0;

  // 将地址、功能码和数据部分打包到 buffer 中
  buffer[index++] = modbusData->address;
  buffer[index++] = modbusData->function;

  for (uint8_t i = 0; i < modbusData->length; i++)
  {
    buffer[index++] = modbusData->data[i];
  }

  // 计算 CRC
  uint16_t crc = calculateCRC(modbusData);

  // 将 CRC 的低字节和高字节加入 buffer
  buffer[index++] = crc & 0xFF;        // 低字节
  buffer[index++] = (crc >> 8) & 0xFF; // 高字节

  // 调用父类的 send 函数来发送整个打包的帧
  RS485Communication::send((const char *)buffer, index);

  // 调试输出：打印发送的数据
  console.log(sqINFO, "Sending Modbus Frame:");
  console.log(sqINFO, buffer, index);
}

// 接收 Modbus 数据帧
size_t ModbusCommunication::receive_modbus(modbus_data_t *modbusData, size_t length, int timeout)
{
  enableReceive(); // 启用接收模式

  // Modbus 帧的总大小：地址(1字节) + 功能码(1字节) + 数据长度 + CRC(2字节)
  uint8_t buffer[MODBUS_MAX_DATA_LENGTH + 4];
  length = RS485Communication::receive((char *)buffer, length, timeout);

  // 调用父类的 receive 函数接收整个 Modbus 帧
  if (length < 4)
  {
    // 如果接收失败，返回 false
    console.log(sqINFO, "No data received from Modbus.");
    return 0;
  }
  console.log(sqINFO, "Received Modbus length:" + String(length));
  console.log(sqINFO, "Received Modbus Frame:");
  // 调试输出：打印接收的数据
  console.log(sqINFO, buffer, length);
  // 将接收到的数据解析为 modbus_data_t 结构
  modbusData->address = buffer[0];
  modbusData->function = buffer[1];
  modbusData->length = length - 4;

  memcpy(modbusData->data, &buffer[2], modbusData->length);

  // 解析接收的 CRC
  uint16_t received_crc = buffer[length - 2] | (buffer[length - 1] << 8);

  console.log(sqDEBUG, "Received CRC: ");
  console.log(sqDEBUG, (uint8_t *)&received_crc, 2);
  uint16_t calculated_crc = calculateCRC(modbusData);
  console.log(sqDEBUG, "\n Calculated CRC: ");
  console.log(sqDEBUG, (uint8_t *)&calculated_crc, 2);
  // 计算 CRC 并验证
  return (calculated_crc == received_crc); // 返回是否 CRC 校验通过
}

// 私有方法：计算整个 Modbus 帧的 CRC
uint16_t ModbusCommunication::calculateCRC(const modbus_data_t *modbusData)
{
  uint16_t crc = 0xFFFF;

  // 计算地址和功能码的 CRC
  crc ^= modbusData->address;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (crc & 0x0001)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc >>= 1;
  }

  crc ^= modbusData->function;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (crc & 0x0001)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc >>= 1;
  }

  // 计算数据部分的 CRC
  for (uint8_t i = 0; i < modbusData->length; i++)
  {
    crc ^= modbusData->data[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}

void ModbusCommunication::print_data(modbus_data_t *modbusData)
{
  // 打印 Modbus 地址和功能码
  Serial.print("Address: ");
  if (modbusData->address < 16)
  {
    Serial.print("0");
  }
  Serial.println(modbusData->address, HEX);
  Serial.print("Function: ");
  if (modbusData->function < 16)
  {
    Serial.print("0");
  }
  Serial.println(modbusData->function, HEX);

  // 打印数据部分
  Serial.print("Data: ");
  print_bytes((uint8_t *)modbusData->data, modbusData->length);

  // 计算并打印 CRC 校验值
  Serial.print("Calculated CRC: ");
  uint16_t calculated_crc = calculateCRC(modbusData);
  print_bytes((uint8_t *)&calculated_crc, 2);
}

#endif