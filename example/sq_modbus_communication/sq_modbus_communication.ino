#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/rs485_communication.h"

RS485Communication rs485Comm;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  RS485Communication rs485Comm(Serial1, 9600, pRS485_RO, pRS485_DI, pRS485_DE, pVext);
  rs485Comm.begin();
}

void loop()
{
  modbus_t mb;
  mb.address = 1;
  mb.function = 3; // Function code for reading registers

  // Correct initialization of data array
  uint8_t modbusData[] = {0x03, 0x04, 0x00, 0x09};
  memcpy(mb.data, modbusData, sizeof(modbusData)); // Copy data into mb.data

  mb.length = sizeof(modbusData); // Set length to the actual number of bytes

  RS485Communication rs485Comm;
  rs485Comm.send_modbus(&mb); // Send the Modbus data
  // rs485Comm.receive_modbus(&mb); // Uncomment if you want to receive after sending

  delay(1000);
}
