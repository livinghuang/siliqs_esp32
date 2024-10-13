#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE

#include "at_command_service.h"
#include <LittleFS.h> // Include the LittleFS library

// 重启设备的功能
void ATCommandService::showInfo()
{
  this->sendResponse("Device information...\r\n");

  // Get and display CPU information
  this->sendResponse("CHIP Information:");
  this->sendResponse("CHIP ID: " + String(get_chip_id()));

#ifdef USE_LORAWAN
  this->sendResponse("LoRaWAN Information:");
  this->sendResponse("devEUI: ");
  this->sendResponse("devAddr: ");
  this->sendResponse("OTAA: ");
  this->sendResponse("appKey: ");
  this->sendResponse("nwkSKey: ");
  this->sendResponse("appSKey: ");
  this->sendResponse("LoRaWAN Class: ");
  this->sendResponse("LoRaWAN Region: ");
#else
  this->sendResponse("No LoRaWAN support");
#endif

#ifdef USE_WIFI_CLIENT
  this->sendResponse("WIFI Information:");
  this->sendResponse("WEB SERVER: 192.168.3.1");
  this->sendResponse("WIFI SSID: ");
  this->sendResponse("WIFI PASSWORD: ");
#else
  this->sendResponse("No WIFI support");
#endif

  // Get and display storage information
  size_t totalBytes = LittleFS.totalBytes(); // Total storage in bytes
  size_t usedBytes = LittleFS.usedBytes();   // Used storage in bytes
  size_t freeBytes = totalBytes - usedBytes; // Free storage in bytes

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
处理 ATLF 命令，List all files in LittleFS
*/
void ATCommandService::listFiles(const String &param)
{
  File root = LittleFS.open("/"); // 打開根目錄

  if (!root || !root.isDirectory())
  {
    this->sendResponse("Failed to open directory");
    return;
  }

  this->sendResponse("Files in LittleFS:");
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
  File file = LittleFS.open(file_name, "r");
  if (!file)
  {
    this->sendResponse("Failed to open file for reading");
    return;
  }

  // 获取文件大小
  size_t fileSize = file.size();
  this->sendResponse("Reading file: " + file_name + ", Size: " + String(fileSize) + " bytes\n");

  // 逐步读取文件内容并发送
  const size_t bufferSize = BLE_MESSAGE_MAX_SIZE; // 每次读取的字节数
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
处理 ATWF 命令，用于写入文件信息
if param = file_param, if index =0 , got header , means to open the file. write header into LittleFS
if param = file_param, if index =1 , got content , means to write the file. write content into LittleFS
*/
void ATCommandService::writeFile(const String &param)
{
  union file_param fileData;
  // 假設 param 是經過轉換得到的 file_param 結構
  memcpy(&fileData, param.c_str(), sizeof(file_param));

  if (fileData.header.index == 0)
  {
    // 寫入文件頭
    File file = LittleFS.open(fileData.header.file_name, "w");
    if (!file)
    {
      this->sendResponse("Failed to open file for writing");
      return;
    }
    this->sendResponse("Writing file header: " + String(fileData.header.file_name) + ", size: " + String(fileData.header.file_size) + "\n");
    file.close();
  }
  else
  {
    // 寫入文件內容
    File file = LittleFS.open(fileData.header.file_name, "a");
    if (file)
    {
      file.seek((fileData.content.index - 1) * sizeof(fileData.content.raw_data));
      file.write((const uint8_t *)fileData.content.raw_data, sizeof(fileData.content.raw_data));
      this->sendResponse("Writing content, index:" + String(fileData.content.index) + "\n");
      file.close();
    }
  }
}

#endif