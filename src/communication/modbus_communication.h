#include "bsp.h"
#ifdef USE_MODBUS
#ifndef MODBUS_COMMUNICATION_H
#define MODBUS_COMMUNICATION_H

#include "rs485_communication.h" // 基于 RS485 的通信类

#define MODBUS_MAX_DATA_LENGTH 255

typedef struct
{
  uint8_t address;                      // Modbus 设备地址
  uint8_t function;                     // Modbus 功能码
  uint8_t data[MODBUS_MAX_DATA_LENGTH]; // Modbus 数据
  uint16_t length;                      // 数据长度
} modbus_data_t;

class ModbusCommunication : public RS485Communication
{
public:
  // 构造函数
  ModbusCommunication(HardwareSerial &serial = Serial1, int baudRate = DEFAULT_RS485_BAUD_RATE, int RO = pRS485_RO, int DI = pRS485_DI, int directionPin = pRS485_DE, int powerPin = -1);

  // 发送 Modbus 数据帧
  void send_modbus(const modbus_data_t *modbusData);

  // 接收 Modbus 数据帧
  size_t receive_modbus(modbus_data_t *modbusData, size_t length = MODBUS_MAX_DATA_LENGTH, int timeout = 1000);

  // 打印 Modbus 数据
  void print_data(modbus_data_t *modbusData);
  // 私有方法：计算整个 Modbus 帧的 CRC
private:
  // 计算 Modbus 数据帧的 CRC 校验
  uint16_t calculateCRC(const modbus_data_t *modbusData);
};

#endif // MODBUS_COMMUNICATION_H
#endif // USE_MODBUS