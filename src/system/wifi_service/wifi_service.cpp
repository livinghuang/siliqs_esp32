#include "wifi_service.h"

#ifdef USE_WIFI // Compile only if USE_WIFI is enabled

WiFiService::WiFiService(const char *ssid, const char *password)
    : ssid(ssid), password(password), wifiTaskHandle(nullptr) {}

// Static function that runs in a FreeRTOS task
void WiFiService::wifiTask(void *pvParameters)
{
  WiFiService *self = static_cast<WiFiService *>(pvParameters);
  WiFi.begin(self->ssid, self->password);

  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.print("Connecting to WiFi");
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(1000);
        Serial.print(".");
      }
      Serial.println("\nConnected to WiFi");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void WiFiService::begin()
{
  if (wifiTaskHandle == nullptr)
  {
    xTaskCreatePinnedToCore(
        wifiTask,        // Task function
        "WiFi Task",     // Task name
        4096,            // Stack size in bytes
        this,            // Task input parameter (class instance)
        1,               // Task priority
        &wifiTaskHandle, // Task handle
        1                // Core to run the task on
    );
  }
}

bool WiFiService::isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void WiFiService::stop()
{
  if (wifiTaskHandle != nullptr)
  {
    vTaskDelete(wifiTaskHandle); // Delete the task
    wifiTaskHandle = nullptr;
  }
  WiFi.disconnect(); // Disconnect from Wi-Fi
  Serial.println("Wi-Fi stopped and disconnected");
}

#endif // USE_WIFI