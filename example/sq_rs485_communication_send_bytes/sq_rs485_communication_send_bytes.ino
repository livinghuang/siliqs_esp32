#include "bsp.h"
#include "siliqs_esp32.h"
#include "communication/rs485_communication.h"
/*
If you use SQC-485I development board, the pRS485_DE pin is connected to the esp32 user pin (aka GPIO9).
When you upload the code, GPIO9 may latch by USB to UART chip (aka CH340K), you could re-plug the USB cable to fix it.
*/

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
  char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

  rs485Comm.send(data, 6);
  Serial.print("Send: ");
  print_bytes((uint8_t *)data, 6);
  delay(1000);
}