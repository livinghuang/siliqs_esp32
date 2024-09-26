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
  char rs485_rxpacket[30]; // 确保这个变量正确声明
  rs485Comm.send("Hello world");
  rs485Comm.receive(rs485_rxpacket, 30);
  Serial.println(rs485_rxpacket);
}
