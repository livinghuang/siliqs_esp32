#include "bsp.h"
#pragma once
#ifdef USE_LORA // Only compile when USE_LORA is enabled

#include "siliqs_heltec_esp32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <RadioLib.h>

typedef struct lora_params_settings
{
  int DIO1;            // DIO1 引脚号
  int BUSY;            // BUSY 引脚号
  int NRST;            // 复位引脚号
  int MISO;            // MISO 引脚号
  int MOSI;            // MOSI 引脚号
  int SCK;             // SCK 引脚号
  int NSS;             // NSS (片选) 引脚号
  float FREQUENCY;     // 频率 (MHz)
  float BANDWIDTH;     // 带宽 (kHz)
  int SF;              // 扩频因子 (Spreading Factor)
  int CR;              // 编码率 (Coding Rate)
  uint8_t SYNC_WORD;   // 同步字 (8 位)
  int OUTPUT_POWER;    // 输出功率 (dBm)
  int PREAMBLE_LENGTH; // 前导码长度 (symbols)
} lora_params_settings;

class LoRaService
{
public:
  // Constructor
  LoRaService(lora_params_settings *params);

  // Initialize the LoRa module and start the background task
  bool begin();

  // Send a message
  void sendMessage(const String &message);

  // Retrieve the last received message
  String getReceivedMessage();

  // Stop the background task
  void stop();

  SX1262 radio;

private:
  // LoRa parameters
  lora_params_settings *params;

  // LoRa module
  Module radioModule;

  // FreeRTOS task handle
  TaskHandle_t taskHandle;

  // Mutex to protect access to shared resources
  SemaphoreHandle_t messageMutex;

  // Flags and buffers
  volatile bool operationDone;
  volatile int transmissionState;
  String messageToSend;
  String receivedMessage;
  bool isTransmitting;

  // LoRa service task function (runs in background)
  static void loRaTask(void *parameter);

  // Internal interrupt handler
  static void IRAM_ATTR onDio1Interrupt(void);

  // Process the result of completed LoRa operations
  void handleOperation();
};

#endif // USE_LORA