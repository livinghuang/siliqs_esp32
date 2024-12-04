#include "sensor_measurement.h"

// Start the sensor task with the specified delay and optional sensor bus mutex
void Sensor::start(int delayMs, SemaphoreHandle_t *busMutex)
{
  taskDelayMs = delayMs;     // Store the delay value in milliseconds
  sensorBusMutex = busMutex; // Store the optional sensor bus mutex

  if (taskHandle == nullptr) // If the task is not already running
  {
    // Create the sensor task with a stack size of 4096 bytes and priority of 1
    if (xTaskCreate(sensorTask, "SensorTask", 4096, this, 1, &taskHandle) != pdPASS)
    {
      Serial.println("Error: Failed to create sensor task");
    }
  }
  else
  {
    Serial.println("Sensor task is already running");
  }
}

// Stop the running sensor task
void Sensor::stop()
{
  if (taskHandle != nullptr) // If the task is currently running
  {
    vTaskDelete(taskHandle); // Delete the task
    taskHandle = nullptr;    // Set the task handle to nullptr to indicate no task is active
  }
}

// FreeRTOS task function that performs periodic sensor measurements
void Sensor::sensorTask(void *parameter)
{
  // Retrieve the Sensor object instance passed as a parameter
  Sensor *sensor = static_cast<Sensor *>(parameter);
  sensor->begin(); // Initialize the sensor (to be implemented in the derived class)

  for (;;)
  {
    // Lock the message mutex to ensure thread-safe access to shared resources
    if (sensor->messageMutex != nullptr)
    {
      xSemaphoreTake(sensor->messageMutex, portMAX_DELAY);
    }

    // If a sensor bus mutex is provided, lock it for safe access to the bus
    if (sensor->sensorBusMutex && xSemaphoreTake(*sensor->sensorBusMutex, portMAX_DELAY) == pdTRUE)
    {
      sensor->getMeasurement();                // Perform a sensor measurement
      xSemaphoreGive(*sensor->sensorBusMutex); // Release the sensor bus mutex
    }
    else if (!sensor->sensorBusMutex) // If no mutex is provided, proceed without locking
    {
      sensor->getMeasurement(); // Perform a sensor measurement directly
    }

    // Unlock the message mutex
    if (sensor->messageMutex != nullptr)
    {
      xSemaphoreGive(sensor->messageMutex);
    }

    // Delay the task for the specified interval before the next measurement
    vTaskDelay(pdMS_TO_TICKS(sensor->taskDelayMs));
  }
}

// Swap the bytes of a 16-bit integer (used for endianness conversion)
void swap_bytes(uint16_t *value)
{
  *value = (*value << 8) | (*value >> 8); // Shift the lower byte to the upper byte and vice versa
}

// Print a byte array in hexadecimal format
void print_bytes(const uint8_t *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    // Add a leading zero for single-digit hex values for better readability
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // Print a newline after the byte array
}

// Calculate a simple checksum by summing all bytes except the last two
uint16_t raw_sum(uint8_t *buffer, int len)
{
  uint16_t sum = 0;
  for (int i = 0; i < len - 2; i++) // Exclude the last two bytes from the sum
  {
    sum += buffer[i];
  }
  return sum;
}
