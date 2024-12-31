#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE

#include "at_command_service.h"
#include "littleFS_src/sqLittleFS.h"
// 重启设备的功能
void ATCommandService::showInfo()
{
  this->sendResponse("Device information...\r\n");

  // Get and display CPU information
  this->sendResponse("CHIP Information:");
  this->sendResponse("CHIP ID: " + String(get_chip_id()));

#ifdef USE_WIFI_CLIENT
  this->sendResponse("WIFI Information:");
  this->sendResponse("WEB SERVER: 192.168.3.1");
  this->sendResponse("WIFI SSID: ");
  this->sendResponse("WIFI PASSWORD: ");
#else
  this->sendResponse("No WIFI support");
#endif

  // Get and display storage information
  size_t totalBytes = sqLittleFS.totalBytes(); // Total storage in bytes
  size_t usedBytes = sqLittleFS.usedBytes();   // Used storage in bytes
  size_t freeBytes = totalBytes - usedBytes;   // Free storage in bytes

  this->sendResponse("Storage Information:");
  this->sendResponse("Total Space: " + String(totalBytes) + " bytes");
  this->sendResponse("Used Space: " + String(usedBytes) + " bytes");
  this->sendResponse("Free Space: " + String(freeBytes) + " bytes");
}

// 重启设备的功能
void ATCommandService::resetDevice()
{
  this->sendResponse("Device is resetting...\r\n");
  delay(1000);   // 延迟 1 秒，以便发送响应后再重启
  ESP.restart(); // 使用 ESP32 的重启功能
}
// 获取所有注册的命令列表
void ATCommandService::getCommandList()
{
  // 遍历 commandCallbacks map，逐个发送命令
  for (const auto &command : commandCallbacks)
  {
    this->sendResponse(command.first + "\r\n"); // 逐个发送命令
  }
}
// 处理 ATE 命令，用于控制回显
void ATCommandService::handleATECommand(const String &param)
{
  if (param == "0")
  {
    echoEnabled = false;
    this->sendResponse("Echo OFF\r\n");
  }
  else if (param == "1")
  {
    echoEnabled = true;
    this->sendResponse("Echo ON\r\n");
  }
  else
  {
    this->sendResponse("ERROR: Invalid ATE parameter\r\n");
  }
}

struct file_content
{
  uint32_t index; // 包类型 : header or content , 0 == header , over 1 is package number
  char raw_data[156];
};

struct file_header
{
  uint32_t index;     // 包类型 : header or content , 0 == header , over 1 is package number
  uint32_t file_size; // 文件大小
  char file_name[32]; // 文件名
  char reserved[sizeof(file_content) - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(char) * 32];
};
union file_param
{
  struct file_header header;
  struct file_content content;
};
/*
处理 ATLF 命令，List all files in sqLittleFS
*/
void ATCommandService::listFiles(const String &param)
{
  File root = sqLittleFS.open("/"); // 打開根目錄

  if (!root || !root.isDirectory())
  {
    this->sendResponse("Failed to open directory");
    return;
  }

  this->sendResponse("Files in sqLittleFS:");
  File file = root.openNextFile();
  while (file)
  {
    this->sendResponse("File: /" + String(file.name()) + ", Size: " + String(file.size()) + "\n");
    file = root.openNextFile();
  }
}
/*
处理 ATRF 命令，用于获取文件信息
param = file_name
*/
void ATCommandService::readFile(const String &param)
{
  String file_name = param;
  Serial.println("Reading file from: " + file_name);

  // 打开文件以读取
  File file = sqLittleFS.open(file_name, "r");
  if (!file)
  {
    this->sendResponse("Failed to open file for reading");
    return;
  }

  // 获取文件大小
  size_t fileSize = file.size();
  this->sendResponse("Reading file: " + file_name + ", Size: " + String(fileSize) + " bytes\n");

  // 逐步读取文件内容并发送
  const size_t bufferSize = AT_COMMAND_MESSAGE_MAX_SIZE; // 每次读取的字节数
  char buffer[bufferSize];

  while (file.available())
  {
    // 读取文件内容
    size_t bytesRead = file.readBytes(buffer, bufferSize);

    // 将文件内容发送回去
    String content = "";
    for (size_t i = 0; i < bytesRead; i++)
    {
      content += buffer[i];
    }

    this->sendResponse(content);          // 发送文件内容
    vTaskDelay(100 / portTICK_PERIOD_MS); // 延迟以避免BLE堆栈过载
  }

  // 关闭文件
  file.close();
}

/*
处理 ATDF 命令，用于删除文件
param = file_name
*/
void ATCommandService::deleteFile(const String &param)
{
  String file_name = param;
  Serial.println("Deleting file: " + file_name);

  // 检查文件是否存在
  if (!sqLittleFS.exists(file_name))
  {
    this->sendResponse("File does not exist: " + file_name);
    return;
  }

  // 删除文件
  if (sqLittleFS.remove(file_name))
  {
    this->sendResponse("File deleted successfully: " + file_name);
  }
  else
  {
    this->sendResponse("Failed to delete file: " + file_name);
  }
}

/*
处理 ATWF 命令，用于写入文件信息
if param = file_param, if index =0 , got header , means to open the file. write header into sqLittleFS
if param = file_param, if index =1 , got content , means to write the file. write content into sqLittleFS
*/
void ATCommandService::writeFile(const String &param)
{
}

/*
处理 ATOTA 命令，用于启动 ATOTA 服务
*/
#include "system/ota_service/ota_service.h"
void ATCommandService::OTAServer(const String &param)
{
  this->sendResponse(server_param_receive(param));
}

#ifdef USE_WEB_OTA_SERVER
void ATCommandService::startWebOTAServer(const String &param)
{
  const char *defaultPassword = "siliqs.net"; // Default password (at least 8 characters)
  char *wifiPassword = nullptr;

  if (param.length() < 8)
  {
    this->sendResponse("Password is short, must be at least 8 characters, set to default");
    console.log(sqINFO, "Password is short, must be at least 8 characters, set to default");
    // Use the default password (no need for dynamic allocation here)
    wifiPassword = const_cast<char *>(defaultPassword); // Casting to char* for compatibility
  }
  else
  {
    // Dynamically allocate memory for the password if it's valid
    wifiPassword = new char[param.length() + 1];              // Allocate memory for the password
    strncpy(wifiPassword, param.c_str(), param.length() + 1); // Copy the password
  }

  static char ssid[20];
  // Seed the random number generator with the ESP's MAC address
  randomSeed(ESP.getEfuseMac() + millis());

  // Generate a random number between 100000 and 999999 for the SSID
  unsigned long randomNum = random(100000, 999999);

  // Format the SSID using the random number
  snprintf(ssid, sizeof(ssid), "SQ-%06lu", randomNum);

  // Create a structure to hold both SSID and password
  WebOTATaskParams *taskParams = new WebOTATaskParams;
  taskParams->ssid = ssid;
  taskParams->password = wifiPassword;

  this->sendResponse("Web OTA Server started...");
  this->sendResponse("SSID:" + String(ssid) + " Password:" + String(wifiPassword));
  // Create a FreeRTOS task to run the web server in the background
  xTaskCreate(
      WebOTAServerTask,       // Task function
      "WebOTAServerTask",     // Name of the task
      10000,                  // Stack size (in bytes)
      (void *)taskParams,     // Task input parameter (the password)
      1,                      // Task priority
      &webOTAServerTaskHandle // Task handle
  );
}
#endif
#endif