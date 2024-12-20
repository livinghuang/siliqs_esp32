#include "bsp.h"
#include "siliqs_esp32.h"
#include "communication/rs485_communication.h"
#include "communication/modbus_communication.h"
ModbusCommunication modbusComm(Serial1, 9600, pRS485_RO, pRS485_DI, pRS485_DE, pVext, false);

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup(SQ_INFO);
  modbusComm.begin();
}

void loop()
{
  // 初始化 Modbus 数据
  modbus_data_t modbusData;

  // 设置数据部分
  uint8_t modbusPayload0[] = {0x00, 0x00, 0x00, 0x0A};
  uint8_t modbusPayload1[] = {0x03, 0x04, 0x00, 0x09};
  uint8_t modbusPayload2[] = {0x05, 0x04, 0x00, 0x14};
  uint8_t modbusPayload3[] = {0x02, 0x00, 0x00};

  static uint8_t send_modbus_command_selector = 0;
  switch (send_modbus_command_selector)
  {
  case 0:
    modbusData.address = 0x01;
    modbusData.function = 0x03;
    memcpy(modbusData.data, modbusPayload0, sizeof(modbusPayload0));
    modbusData.length = sizeof(modbusPayload0);
    send_modbus_command_selector = 1;
    break;
  case 1:
    modbusData.address = 0x01;
    modbusData.function = 0x03;
    memcpy(modbusData.data, modbusPayload1, sizeof(modbusPayload1));
    modbusData.length = sizeof(modbusPayload1);
    send_modbus_command_selector = 2;
    break;
  case 2:
    modbusData.address = 0x01;
    modbusData.function = 0x03;
    memcpy(modbusData.data, modbusPayload2, sizeof(modbusPayload2));
    modbusData.length = sizeof(modbusPayload2);
    send_modbus_command_selector = 3;
    break;
  case 3:
    modbusData.address = 0x10;
    modbusData.function = 0x21;
    memcpy(modbusData.data, modbusPayload3, sizeof(modbusPayload3));
    modbusData.length = sizeof(modbusPayload3);
    send_modbus_command_selector = 0;
    break;
  }
  modbusComm.send_modbus(&modbusData);

  if (modbusComm.receive_modbus(&modbusData) > 0)
  {
    Serial.println("Modbus 数据接收成功");
    modbusComm.print_data(&modbusData);
  }
  else
  {
    Serial.println("Modbus 数据接收失败");
  }
  delay(100);
}
