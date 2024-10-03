#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/lora_communication.h"

LoraCommunication loraComm;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  loraComm.begin(); // 初始化 LoRa 设备
}

void loop()
{
  // 发送数据包
  loraComm.send("Hello world");

  // 接收数据包
  char buffer[BUFFER_SIZE];
  loraComm.receive(buffer, BUFFER_SIZE);

  // 打印接收到的数据
  Serial.println(String(buffer));

  delay(1000); // 根据应用需求调整延迟
}
