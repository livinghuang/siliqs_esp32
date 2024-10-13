#include "bsp.h"
#include "siliqs_heltec_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */

// Create an instance of the LoRaWAN service class
SQ_LoRaWanService loraService;

// The setup function runs once when the ESP32 starts
void setup()
{
  // Initialize serial for debugging purposes
  siliqs_heltec_esp32_setup();

  // Start the LoRaWAN task
  Serial.println("Starting LoRaWAN service...");
  loraService.startTask(); // Start the FreeRTOS task for LoRaWAN processing
}

// The loop function runs over and over again
void loop()
{
}