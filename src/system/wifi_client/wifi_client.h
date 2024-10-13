#pragma once
#include "bsp.h"       // Include user settings like USE_WIFI_CLIENT
#ifdef USE_WIFI_CLIENT // Compile only if USE_WIFI_CLIENT is enabled
#include <WiFi.h>

class WiFiService
{
private:
  const char *ssid;                         // Wi-Fi SSID
  const char *password;                     // Wi-Fi Password
  TaskHandle_t wifiTaskHandle;              // Handle for the Wi-Fi task
  static void wifiTask(void *pvParameters); // Static method for the task

public:
  WiFiService(const char *ssid, const char *password);
  void begin();       // Starts Wi-Fi connection task
  bool isConnected(); // Checks if the device is connected
  void stop();        // Stops the Wi-Fi task and disconnects Wi-Fi
};
#endif // USE_WIFI_CLIENT