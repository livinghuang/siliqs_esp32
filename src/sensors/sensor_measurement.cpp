#include "sensor_measurement.h"

void Sensor::start(int delayMs, SemaphoreHandle_t *busMutex)
{
  taskDelayMs = delayMs; // store delay value in ms
  sensorBusMutex = busMutex;
  if (taskHandle == nullptr)
  {
    xTaskCreate(sensorTask, "SensorTask", 4096, this, 1, &taskHandle);
  }
}

void Sensor::stop()
{
  if (taskHandle != nullptr)
  {
    vTaskDelete(taskHandle);
    taskHandle = nullptr;
  }
}

void Sensor::sensorTask(void *parameter)
{
  Sensor *sensor = static_cast<Sensor *>(parameter);
  sensor->begin(); // Initialize the sensor

  for (;;)
  {
    xSemaphoreTake(sensor->messageMutex, portMAX_DELAY); // Protect shared data

    // 锁定 sensor bus 互斥锁以安全访问  sensor bus 总线
    if (xSemaphoreTake(*sensor->sensorBusMutex, portMAX_DELAY) == pdTRUE)
    {
      sensor->getMeasurement();                // 从传感器读取数据
      xSemaphoreGive(*sensor->sensorBusMutex); // 释放 sensor bus 互斥锁
    }
    xSemaphoreGive(sensor->messageMutex);
    vTaskDelay(pdMS_TO_TICKS(sensor->taskDelayMs)); // Adjust delay as needed
  }
}

void swap_bytes(uint16_t *value)
{
  *value = (*value << 8) | (*value >> 8);
}
void print_bytes(const uint8_t *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    // Print each byte in hexadecimal format with leading zeros
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // Print a newline character at the end
}

uint16_t raw_sum(uint8_t *buffer, int len)
{
  uint16_t sum = 0;
  for (int i = 0; i < len - 2; i++)
  {
    sum += buffer[i];
  }
  return sum;
}