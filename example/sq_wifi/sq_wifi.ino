#include "bsp.h"
#include "siliqs_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";

WiFiService wifi(ssid, password);

void setup()
{
  Serial.begin(115200);

  // Start the Wi-Fi connection
  wifi.begin();
}

void loop()
{
  if (wifi.isConnected())
  {
    Serial.println("ESP32 is connected to Wi-Fi");
  }
  else
  {
    Serial.println("ESP32 is not connected to Wi-Fi");
  }

  delay(5000); // Wait 5 seconds before checking again
}