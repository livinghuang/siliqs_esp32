#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE

#include "at_command_service.h"

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

#ifdef USE_WIFI
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
#endif