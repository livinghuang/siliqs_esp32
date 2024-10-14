#include "bsp.h"
#ifdef USE_WEB_OTA_SERVER
#include "web_ota_server.h"
#include "web_ota_server_html.h" // Assume this contains INDEX_HTML for the web page

// Constructor: initialize with Wi-Fi credentials
WebOTAServerClass::WebOTAServerClass(const char *ssid, const char *password)
    : server(80), local_IP(192, 168, 4, 1), gateway(192, 168, 4, 1), subnet(255, 255, 255, 0), password(password)
{
  if (ssid == nullptr)
  {
    // Use the default SSID
    this->ssid = "SQ-1234";
  }
  else
  {
    // Set the provided SSID
    this->ssid = ssid;
  }

  if (password == nullptr)
  {
    // Use the default password
    this->password = "siliqs.net";
  }
  else
  {
    // Set the provided password
    this->password = password;
  }
}

void WebOTAServerClass::begin()
{
  console.log(sqINFO, "启动 HTTP 服务器:" + String(ssid) + ":" + String(password));
  // Start Wi-Fi as Access Point
  WiFi.softAP(ssid, password);

  // Configure the access point IP
  WiFi.softAPConfig(local_IP, gateway, subnet);

  console.log(sqINFO, "AP IP 地址: " + String(WiFi.softAPIP().toString()));

  // Setup OTA handlers
  setupOTA();

  // Start the server
  server.begin();
  console.log(sqINFO, "HTTP 服务器已启动.");
}

void WebOTAServerClass::setupOTA()
{
  // Serve the upload form at the root
  server.on("/", HTTP_GET, [this]()
            { server.send(200, "text/html", INDEX_HTML); });

  // Handle firmware update on "/update" route
  server.on("/update", HTTP_POST, [this]()
            { handleUpdate(); }, [this]()
            { handleFileUpload(); });
}

void WebOTAServerClass::handleUpdate()
{
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}

void WebOTAServerClass::handleFileUpload()
{
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.setDebugOutput(true);
    console.log(sqINFO, "Update: %s\n", upload.filename.c_str());
    if (!Update.begin())
    { // Start with max available size
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
    { // true to finalize the firmware update
      console.log(sqINFO, "Update Success: %u\nRebooting...\n", upload.totalSize);
    }
    else
    {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
  else
  {
    console.log(sqINFO, "Update Failed: status=%d\n", upload.status);
  }
}

void WebOTAServerClass::runServer()
{
  while (true)
  {
    server.handleClient();               // Process incoming requests
    vTaskDelay(10 / portTICK_PERIOD_MS); // Use vTaskDelay for FreeRTOS tasks
  }
}

// Create an instance of WebOTAServerClass with the default password
WebOTAServerClass *webOTAServer;

// FreeRTOS task handle
TaskHandle_t webOTAServerTaskHandle = NULL;

void WebOTAServerTask(void *parameter)
{
  // Cast the parameter to a pointer of type WebOTATaskParams
  WebOTATaskParams *params = reinterpret_cast<WebOTATaskParams *>(parameter);

  const char *ssid = params->ssid;         // Extract the SSID
  const char *password = params->password; // Extract the password

  if (webOTAServer == nullptr)
  {
    // Dynamically create the WebOTAServerClass object with SSID and password
    webOTAServer = new WebOTAServerClass(ssid, password);
  }

  // Initialize and start the web server
  webOTAServer->begin();

  // Run the server loop
  webOTAServer->runServer();

  // Memory cleanup: Free dynamically allocated memory for SSID and password
  if (params->ssid != nullptr)
  {
    delete[] params->ssid; // Free the SSID memory if it was dynamically allocated
  }

  if (params->password != nullptr && strcmp(params->password, "siliqs.net") != 0)
  {
    delete[] params->password; // Free the password memory if it was dynamically allocated
  }

  delete params; // Free the WebOTATaskParams structure

  vTaskDelete(NULL); // Delete task when finished
}
#endif // USE_WEB_OTA_SERVER