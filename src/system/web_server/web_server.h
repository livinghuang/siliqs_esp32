#pragma once
#include "bsp.h"
#ifdef USE_WEB_SERVER
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Update.h>
#include <Ticker.h>

class WebServerApp
{
public:
  WebServerApp(const char *host);
  ~WebServerApp();

  void begin();        // Function to start the server
  void handleClient(); // Function to handle incoming requests

private:
  const char *host;

  WebServer server; // Web server instance
  Ticker tkSecond;
  uint8_t otaDone;

  void apMode();                        // Setup AP mode
  void setupRoutes();                   // Function to set up routes
  void onUpdate();                      // Function to handle the update process
  void onUpdateEnd();                   // Function to handle update completion
  String generatePass(uint8_t str_len); // Function to generate random password

  void everySecond(); // Function to check OTA progress
};
#endif // USE_WEB_SERVER