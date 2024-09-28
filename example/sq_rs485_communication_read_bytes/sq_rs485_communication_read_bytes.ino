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
  // 从 RS485 接收数据，超时时间为 1000 毫秒
  char buf[6];
  int length = rs485Comm.receive(buf, sizeof(buf));

  if (length == 0)
  {
    Serial.println("No data received.");
    Serial.flush();
    return;
  }
  // 打印接收到的数据
  Serial.print("Received: ");
  print_bytes((uint8_t *)buf, sizeof(buf));
  Serial.println();
}