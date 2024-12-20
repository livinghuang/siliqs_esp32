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
  // Constructor
  ModbusCommunication(HardwareSerial &serial = Serial1, int baudRate = DEFAULT_RS485_BAUD_RATE, int RO = pRS485_RO, int DI = pRS485_DI, int directionPin = pRS485_DE, int powerPin = -1, bool highActive = true);

  // Send Modbus data frame
  void send_modbus(const modbus_data_t *modbusData);

  // Receive Modbus data frame
  size_t receive_modbus(modbus_data_t *modbusData, size_t length = MODBUS_MAX_DATA_LENGTH, int timeout = -1);
  size_t receive_modbus(modbus_data_t *modbusData, char start_char, size_t length = MODBUS_MAX_DATA_LENGTH, int timeout = -1);

  // Print Modbus data
  void print_data(modbus_data_t *modbusData);

  // Start the FreeRTOS task for Modbus slave service
  /**
   * Starts the FreeRTOS task for Modbus slave service.
   * @param callback A user-defined callback for processing Modbus requests.
   * @return True if the task starts successfully, false otherwise.
   */
  bool startSlaveTask(std::function<void(const modbus_data_t *, modbus_data_t *&)> callback = nullptr, char start_char = 0x10, size_t length = 7, int timeout = 100);

private:
  char start_char = 0x10;
  size_t length = 7;
  int timeout = 100;
  // FreeRTOS task handle
  TaskHandle_t modbusSlaveTaskHandle = nullptr;

  // FreeRTOS task function for Modbus slave
  static void modbusSlaveTask(void *parameters);

  // Calculate Modbus data frame CRC
  uint16_t calculateCRC(const modbus_data_t *modbusData);

  // User-defined callback for processing Modbus requests
  std::function<void(const modbus_data_t *, modbus_data_t *&)> requestCallback;
};

#endif // MODBUS_COMMUNICATION_H
#endif // USE_MODBUS