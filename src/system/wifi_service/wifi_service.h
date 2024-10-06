#include "bsp.h" // Include board support package settings

#if USE_WIFI // Only compile the WiFi class if USE_WIFI is enabled

#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>

class WiFi_Communication
{
private:
  const char *ssid;                         // Wi-Fi SSID
  const char *password;                     // Wi-Fi Password
  TaskHandle_t wifiTaskHandle;              // Handle for the Wi-Fi task
  static void wifiTask(void *pvParameters); // Static method for the task

public:
  WiFi_Communication(const char *ssid, const char *password);
  void begin();       // Starts Wi-Fi connection task
  bool isConnected(); // Checks if the device is connected
  void stop();        // Stops the Wi-Fi task and disconnects Wi-Fi
};

#endif // WIFI_COMMUNICATION_H

#endif // USE_WIFI