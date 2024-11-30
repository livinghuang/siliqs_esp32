#include "bsp.h"
#include "siliqs_esp32.h"
#include "communication/rs485_communication.h"

RS485Communication rs485Comm(Serial1, 9600, pRS485_RO, pRS485_DI, pRS485_DE, pVext, false);
void print_bytes(uint8_t *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup(SQ_INFO);
  rs485Comm.begin();
}
void loop()
{
  // 从 RS485 接收数据，超时时间为 1000 毫秒
  char buf[6];
  int length = rs485Comm.receive(buf, sizeof(buf));

  if (length > 0)
  {
    // 打印接收到的数据
    Serial.print("Received: ");
    print_bytes((uint8_t *)buf, sizeof(buf));
  }
}