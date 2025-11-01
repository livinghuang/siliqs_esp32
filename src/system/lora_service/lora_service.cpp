#include "bsp.h"
#define USE_LORA

#ifdef USE_LORA // Only compile when USE_LORA is enabled
#include "lora_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---- 放在某個共用 header (.h) 最上面 ----
#ifndef LORA_TASK_CORE
#if defined(ARDUINO_ARCH_ESP32)
// FreeRTOS 會用這個巨集宣告處理器數
#if (portNUM_PROCESSORS == 1)
// ESP32-C3 / ESP32-S2 等單核
#define LORA_TASK_CORE 0
#else
// ESP32 / ESP32-S3 等雙核，預設放在 APP core(1)
#define LORA_TASK_CORE 1
#endif
#else
// 非 ESP32 平台（若有移植），用 0 當預設
#define LORA_TASK_CORE 0
#endif
#endif

static LoRaService *loRaInstance = nullptr; // Static global instance pointer for interrupt handling

LoRaService::LoRaService(lora_params_settings *params)
    : params(params),
      radioModule(params->NSS, params->DIO1, params->NRST, params->BUSY),
      radio(&radioModule),
      taskHandle(nullptr),
      messageMutex(xSemaphoreCreateMutex()),
      operationDone(false),
      transmissionState(RADIOLIB_ERR_NONE),
      isTransmitting(false)
{
  loRaInstance = this; // Set the static global instance
}

bool LoRaService::begin()
{
  console.log(sqINFO, "[LoRaService] Initializing LoRa module...");

  // Initialize SPI
  SPI.begin(params->SCK, params->MISO, params->MOSI, params->NSS);

  // Initialize the radio with LoRa parameters
  int state = radio.begin(
      params->FREQUENCY,
      params->BANDWIDTH,
      params->SF,
      params->CR,
      params->SYNC_WORD,
      params->OUTPUT_POWER,
      params->PREAMBLE_LENGTH);

  if (state != RADIOLIB_ERR_NONE)
  {
    console.log(sqINFO, "LoRa initialization failed, code: %d\n", state);
    return false;
  }

  console.log(sqINFO, "LoRa module initialized successfully.");

  // Set interrupt for DIO1
  radio.setDio1Action(onDio1Interrupt);

  // // Create the background task
  xTaskCreatePinnedToCore(
      loRaTask,    // Task function
      "LoRaTask",  // Task name
      4096,        // Stack size
      this,        // Task parameter
      1,           // Priority
      &taskHandle, // Task handle
      LORA_TASK_CORE);

  return true;
}

void LoRaService::sendMessage(const String &message)
{
  xSemaphoreTake(messageMutex, portMAX_DELAY);    // Lock mutex
  messageToSend = message;                        // Store the message to be sent
  isTransmitting = true;                          // Set transmitting flag
  int state = radio.startTransmit(messageToSend); // Start transmission
  if (state != RADIOLIB_ERR_NONE)
  {
    console.log(sqINFO, "Failed to start transmission, code: %d\n", state);
  }
  xSemaphoreGive(messageMutex); // Unlock mutex
}

String LoRaService::getReceivedMessage()
{
  xSemaphoreTake(messageMutex, portMAX_DELAY); // Lock mutex
  String message = receivedMessage;            // Copy the last received message
  receivedMessage = "";                        // Clear the received message
  xSemaphoreGive(messageMutex);                // Unlock mutex
  return message;
}

void LoRaService::stop()
{
  if (taskHandle)
  {
    vTaskDelete(taskHandle); // Delete the FreeRTOS task
    taskHandle = nullptr;
  }
  if (messageMutex)
  {
    vSemaphoreDelete(messageMutex); // Delete the mutex
    messageMutex = nullptr;
  }
}

void LoRaService::loRaTask(void *parameter)
{
  LoRaService *self = static_cast<LoRaService *>(parameter);

  while (true)
  {
    if (self->operationDone)
    {
      self->handleOperation(); // Handle completed LoRa operations
      self->operationDone = false;
    }
    vTaskDelay(10); // Avoid busy waiting
  }
}

void IRAM_ATTR LoRaService::onDio1Interrupt()
{
  if (loRaInstance)
  {
    loRaInstance->operationDone = true; // Signal operation completion
  }
}

void LoRaService::handleOperation()
{
  if (isTransmitting)
  {
    // Handle transmission completion
    if (transmissionState == RADIOLIB_ERR_NONE)
    {
      console.log(sqINFO, "[LoRaService] Message sent successfully.");
    }
    else
    {
      console.log(sqINFO, "Transmission failed, code: %d\n", transmissionState);
    }
    radio.startReceive(); // Switch to receive mode
    isTransmitting = false;
  }
  else
  {
    // Handle reception
    String incoming;
    int state = radio.readData(incoming);
    if (state == RADIOLIB_ERR_NONE)
    {
      console.log(sqINFO, "[LoRaService] Message received.");
      xSemaphoreTake(messageMutex, portMAX_DELAY); // Lock mutex
      receivedMessage = incoming;                  // Store received message
      xSemaphoreGive(messageMutex);                // Unlock mutex
    }
    else
    {
      console.log(sqINFO, "Reception failed, code: %d\n", state);
    }
  }
}

#endif