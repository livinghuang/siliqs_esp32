#include "web_server.h"

#ifdef USE_WEB_SERVER
#define PASSWORD "siliqs.net"
const char *SSID_FORMAT = "SQ-%06lX"; // SSID format for AP mode

// Dummy favicon data for example purposes
const uint8_t favicon_ico_gz[] = {/* your favicon data */};
const size_t favicon_ico_gz_len = sizeof(favicon_ico_gz);

// Dummy index HTML for example purposes
const char *indexHtml = "<html><body><h1>ESP32 Web Server</h1></body></html>";

WebServerApp::WebServerApp(const char *host)
    : server(80), host(host), otaDone(0)
{
}

WebServerApp::~WebServerApp()
{
  server.stop();
  WiFi.disconnect();
}

// Setup the Access Point mode
void WebServerApp::apMode()
{
  char ssid[13];
  char passwd[] = PASSWORD;
  long unsigned int espmac = ESP.getEfuseMac() >> 24;
  snprintf(ssid, 13, SSID_FORMAT, espmac);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passwd); // Set up the SoftAP
  MDNS.begin(host);

  Serial.printf("AP: %s, PASS: %s\n", ssid, passwd);
}

// Function to handle the update process
void WebServerApp::onUpdate()
{
  size_t fsize = UPDATE_SIZE_UNKNOWN;
  if (server.hasArg("size"))
  {
    fsize = server.arg("size").toInt();
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.printf("Receiving Update: %s, Size: %d\n", upload.filename.c_str(), fsize);
    if (!Update.begin(fsize))
    {
      otaDone = 0;
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
    {
      Update.printError(Serial);
    }
    else
    {
      otaDone = 100 * Update.progress() / Update.size();
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
    {
      Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
    }
    else
    {
      Serial.printf("%s\n", Update.errorString());
      otaDone = 0;
    }
  }
}

// Function to handle update completion
void WebServerApp::onUpdateEnd()
{
  server.sendHeader("Connection", "close");
  if (Update.hasError())
  {
    server.send(502, "text/plain", Update.errorString());
  }
  else
  {
    server.sendHeader("Refresh", "10");
    server.sendHeader("Location", "/");
    server.send(307);
    ESP.restart();
  }
}

// Function to set up web server routes
void WebServerApp::setupRoutes()
{
  server.on(
      "/update", HTTP_POST,
      [this]()
      { onUpdateEnd(); },
      [this]()
      { onUpdate(); });

  // Add a favicon handler
  // server.on("/favicon.ico", HTTP_GET, [this]()
  //           {
  //       server.sendHeader("Content-Encoding", "gzip");
  //       server.send_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len); });

  // Default route
  server.onNotFound([this]()
                    {
                      server.send(200, "text/html", indexHtml); // Serve the index page
                    });

  server.begin();
  Serial.printf("Web Server ready at http://esp32.local or http://%s\n", WiFi.softAPIP().toString().c_str());
}

// Function to start the web server
void WebServerApp::begin()
{
  apMode();      // Set the device to AP mode
  setupRoutes(); // Set up the routes

  // Start the periodic check for OTA progress
  tkSecond.attach(1, [this]()
                  { everySecond(); });
}

// Function to handle incoming client requests
void WebServerApp::handleClient()
{
  server.handleClient();
}

// Function to check OTA progress every second
void WebServerApp::everySecond()
{
  if (otaDone > 1)
  {
    Serial.printf("OTA Progress: %d%%\n", otaDone);
  }
}

#endif // USE_WEB_SERVER