#include "bsp.h"
#ifdef USE_LORA
#ifndef LORA_COMMUNICATION_H
#define LORA_COMMUNICATION_H
#include "siliqs_heltec_esp32.h"
#include "communication.h"
#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 5       // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // 前导码长度
#define LORA_SYMBOL_TIMEOUT 0   // 符号超时
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE 30 // 定义数据包大小

class LoraCommunication : public Communication
{
public:
  // 构造函数
  LoraCommunication();

  // 实现抽象类的虚函数
  void begin() override;                                        // 初始化LoRa设备
  void send(const char *data) override;                         // 发送数据包
  void receive(char *buffer, int length, int timeout) override; // 接收数据包

  // 发射完成的静态回调函数
  static void OnTxDone();

  // 接收完成的静态回调函数
  static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

private:
  static char txpacket[BUFFER_SIZE]; // 修改为静态变量
  static char rxpacket[BUFFER_SIZE]; // 修改为静态变量
  static bool lora_idle;

  // 初始化 Radio 事件
  static RadioEvents_t RadioEvents;
};

#endif // LORA_COMMUNICATION_H
#endif // USE_LORA