#include "bsp.h"
#include "siliqs_heltec_esp32.h"
/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */

void setup()
{
  Serial.begin(115200);
  siliqs_heltec_esp32_setup(SQ_INFO);
  WebOTATaskParams *taskParams = new WebOTATaskParams;
  taskParams->ssid = "SQ-888888";
  taskParams->password = "12345678";

  xTaskCreate(
      WebOTAServerTask,       // Task function
      "WebOTAServerTask",     // Name of the task
      10000,                  // Stack size (in bytes)
      (void *)taskParams,     // Task input parameter (the password)
      1,                      // Task priority
      &webOTAServerTaskHandle // Task handle
  );
}

void loop()
{
  delay(10);
}