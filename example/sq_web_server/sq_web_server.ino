#include "bsp.h"
#include "siliqs_heltec_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
WebServerApp webServer("esp32-webupdate");
void setup()
{
  Serial.begin(115200);
  Serial.println("Booting Sketch...");
  siliqs_heltec_esp32_setup();
  webServer.begin(); // Start the web server
}

void loop()
{
  webServer.handleClient(); // Handle client requests
  delay(150);               // Allow the CPU to switch to other tasks
}