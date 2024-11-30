#pragma once
#include "bsp.h"
#ifdef USE_WEB_OTA_SERVER
#include "siliqs_esp32.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#define PASSWORD "siliqs.net"

struct WebOTATaskParams
{
  char *ssid;
  char *password;
};

class WebOTAServerClass
{
public:
  WebOTAServerClass(const char *ssid, const char *password);
  void begin();     // Initialize the web server and WiFi
  void runServer(); // Run the web server task (for FreeRTOS)

private:
  WebServer server;
  const char *ssid;
  const char *password;
  IPAddress local_IP;
  IPAddress gateway;
  IPAddress subnet;

  void setupOTA();
  void handleUpdate();
  void handleFileUpload();
};

// Create an instance of WebServerClass with the desired SSID and password
extern WebOTAServerClass *webOTAServer;

// FreeRTOS task handle
extern TaskHandle_t webOTAServerTaskHandle;
void WebOTAServerTask(void *parameter);
#endif // USE_WEB_OTA_SERVER