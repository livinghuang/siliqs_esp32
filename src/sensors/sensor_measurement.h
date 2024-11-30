#pragma once
#include "siliqs_esp32.h"
#include "Arduino.h"
class Sensor
{
public:
  Sensor() : taskHandle(nullptr), messageMutex(xSemaphoreCreateMutex()) {}

  virtual void begin() = 0;          // Initialize the sensor
  virtual void getMeasurement() = 0; // Get the measurement once

  virtual void start(int delayMs = 1000, SemaphoreHandle_t *sensorBusMutex = nullptr); // Start the task for the sensor
  virtual void stop();                                                                 // Stop the task for the sensor

  virtual ~Sensor()
  {
    if (taskHandle != nullptr)
    {
      vTaskDelete(taskHandle); // Ensure the task is deleted
    }
    if (messageMutex != nullptr)
    {
      vSemaphoreDelete(messageMutex); // Delete the mutex
    }
  }

protected:
  int taskDelayMs;
  TaskHandle_t taskHandle;        // FreeRTOS task handle
  SemaphoreHandle_t messageMutex; // Mutex for thread safety
  SemaphoreHandle_t *sensorBusMutex;
  static void sensorTask(void *parameter);
};

void print_bytes(const uint8_t *data, int length);
void swap_bytes(uint16_t *value);
uint16_t raw_sum(uint8_t *buffer, int len);