#include "bsp.h"
#if USE_WIFI // Only compile Wi-Fi implementation if USE_WIFI is enabled
#include "wifi_communication.h"
WiFi_Communication::WiFi_Communication(const char *ssid, const char *password)
    : ssid(ssid), password(password), wifiTaskHandle(nullptr) {}

// Static function that runs in a FreeRTOS task
void WiFi_Communication::wifiTask(void *pvParameters)
{
  WiFi_Communication *self = static_cast<WiFi_Communication *>(pvParameters);
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

void WiFi_Communication::begin()
{
  if (wifiTaskHandle == nullptr)
  {
    xTaskCreatePinnedToCore(
        wifiTask, "WiFi Task", 4096, this, 1, &wifiTaskHandle, 1);
  }
}

bool WiFi_Communication::isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void WiFi_Communication::stop()
{
  if (wifiTaskHandle != nullptr)
  {
    vTaskDelete(wifiTaskHandle);
    wifiTaskHandle = nullptr;
  }
  WiFi.disconnect();
  Serial.println("Wi-Fi stopped and disconnected");
}

#endif // USE_WIFI