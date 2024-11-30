#include "bsp.h"
#include "siliqs_esp32.h"
#include "communication/rs485_communication.h"
#include "communication/modbus_communication.h"

ModbusCommunication modbusComm(Serial1, 4800, pRS485_RO, pRS485_DI, pRS485_DE, pVext);

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup(SQ_DEBUG);
  modbusComm.begin();
}

void loop()
{

  // 初始化 Modbus 数据
  modbus_data_t modbusData;
  // modbusData.address = 0x01;
  // modbusData.function = 0x03;

  // 设置数据部分
  uint8_t modbus_command1[] = {0x03, 0x04, 0x00, 0x09};
  uint8_t modbusResponse1[] = {0x12, 0x00, 0x00, 0x00, 0x31, 0x96, 0x00, 0x00, 0x05, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x01, 0x34, 0x0C, 0x03};

  uint8_t modbus_command2[] = {0x05, 0x04, 0x00, 0x14};
  uint8_t modbusResponse2[] = {0x28, 0x00, 0x00, 0x00, 0x31, 0x96, 0x67, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x04, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x35, 0x0C, 0x03, 0x00, 0x00, 0x52, 0x35, 0x10, 0x20, 0x11, 0x47};

  // memcpy(modbusData.data, modbusPayload, sizeof(modbusPayload));
  // modbusData.length = sizeof(modbusPayload);
  // modbusComm.send_modbus(&modbusData);
  int expectedLength = sizeof(modbus_command1) + 4;
  if (modbusComm.receive_modbus(&modbusData, expectedLength, 1000) > 0)
  {
    Serial.println("Modbus 数据接收成功");
    modbusComm.print_data(&modbusData);
    if (modbusData.address == 0x01 && modbusData.function == 0x03)
    {
      if (memcmp(modbusData.data, modbus_command1, sizeof(modbus_command1)) == 0)
      {
        Serial.println("Modbus command 1 接收成功");
        memcpy(modbusData.data, modbusResponse1, sizeof(modbusResponse1));
        modbusData.length = sizeof(modbusResponse1);
        modbusComm.send_modbus(&modbusData);
      }

      if (memcmp(modbusData.data, modbus_command2, sizeof(modbus_command2)) == 0)
      {
        // Serial.println("Modbus command 2 接收成功");
        memcpy(modbusData.data, modbusResponse2, sizeof(modbusResponse2));
        modbusData.length = sizeof(modbusResponse2);
        modbusComm.send_modbus(&modbusData);
      }
    }
  }
  else
  {
    Serial.println("CRC 校验失败");
  }
}