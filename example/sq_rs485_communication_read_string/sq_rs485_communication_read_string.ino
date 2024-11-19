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
  siliqs_heltec_esp32_setup(SQ_INFO);
  RS485Communication rs485Comm(Serial1, 9600, pRS485_RO, pRS485_DI, pRS485_DE, pVext);
  rs485Comm.begin();
}
void loop()
{
  rs485Comm.setReceiveTimeout(1000);
  String data = rs485Comm.readStringUntil('\n');
  Serial.println(data);
}