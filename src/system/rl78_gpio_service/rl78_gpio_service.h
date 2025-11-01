#pragma once
#include "bsp.h"
#ifdef USE_RL78_GPIO_SERVICE
#include <Arduino.h>
/**
 * @brief Service class for communicating with the SQIO6I2O RL78 board via AT commands.
 * * This class uses a FreeRTOS task to handle Unsolicited Result Codes (URCs)
 * and provides methods to send AT commands.
 */
class rl78_gpio_service
{
public:
  /**
   * @brief Initializes the hardware serial port and starts the communication task.
   * * @param pRx The ESP32 pin connected to the RL78 TX.
   * @param pTx The ESP32 pin connected to the RL78 RX.
   * @param baudrate The communication baudrate (must match RL78 firmware).
   */
  void begin(uint8_t pRx, uint8_t pTx, uint32_t baudrate = 9600)
  {
    this->pRx = pRx;
    this->pTx = pTx;
    this->baudrate = baudrate;

    // Allocate HardwareSerial object dynamically or use a static one if available
    // For simplicity, we'll assume a static allocation is managed outside or
    // use a predefined serial port like Serial1.
    // In a real ESP-IDF/Arduino project, you'd configure the specific Uart.

    // Using Serial2 for demonstration, as Serial0 is Console and Serial1 is often used for other purposes.
    // If you need Serial1, change the global serial object.
    rl78_serial.begin(this->baudrate, SERIAL_8N1, this->pRx, this->pTx);

    xTaskCreatePinnedToCore(
        taskLoop,
        "rl78_gpio_task",
        2048, // Increased stack size for safety
        this,
        1,
        &taskHandle,
        0); // Pin to Core 0
  }

  /**
   * @brief Terminates the communication task and serial port.
   */
  void end()
  {
    if (taskHandle != nullptr)
    {
      vTaskDelete(taskHandle);
      taskHandle = nullptr;
    }
    // rl78_serial.end(); // If end() is available/needed
  }

  /**
   * @brief Helper: sends an AT command and waits for a response (blocking).
   * * @param cmd The AT command string (e.g., "AT+OUT=HHHLLLLL").
   * @param timeout The maximum time in milliseconds to wait for a response.
   */
  void sendAT(const char *cmd, uint32_t timeout = 1000)
  {
    rl78_serial.print(cmd);
    rl78_serial.print("\r\n");
    Serial.print(">> ");
    Serial.println(cmd);

    // This section is non-blocking to the FreeRTOS environment
    // but blocking to the task/caller.
    uint32_t t0 = millis();
    while (millis() - t0 < timeout)
    {
      while (rl78_serial.available())
      {
        char c = rl78_serial.read();
        Serial.write(c); // show on console
      }
      // Give time to other tasks
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    Serial.println();
  }

private:
  HardwareSerial rl78_serial = Serial1; // Use a dedicated hardware serial port
  TaskHandle_t taskHandle = nullptr;
  uint8_t pRx, pTx;
  uint32_t baudrate;

  /**
   * @brief FreeRTOS task function to handle URCs (Unsolicited Result Codes) from RL78.
   * * This task continuously monitors the RL78 serial port for incoming data.
   * @param pvParameters Pointer to the class instance.
   */
  static void taskLoop(void *pvParameters)
  {
    rl78_gpio_service *instance = (rl78_gpio_service *)pvParameters;

    // The main loop for monitoring URCs (similar to the original loop() function)
    for (;;)
    {
      if (instance->rl78_serial.available())
      {
        // Read character-by-character and echo to the debug console
        // In a real application, you would buffer and parse the URC here (e.g., "+IN: HLLHHL")
        char c = instance->rl78_serial.read();
        Serial.write(c);
      }

      // Yield the task if no data is available to prevent watchdog timer from tripping
      // and allow other tasks to run.
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    // Should never be reached, but good practice for completeness
    vTaskDelete(NULL);
  }
};

// Global instance declaration
extern class rl78_gpio_service rl78_gpio_service;

#endif