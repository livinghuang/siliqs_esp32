#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE
#ifndef AT_COMMAND_SERVICE_H
#define AT_COMMAND_SERVICE_H
#include "siliqs_esp32.h"
#include <Arduino.h>
#include <functional>
#include <map>
#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define AT_COMMAND_MESSAGE_MAX_SIZE 1024

class ATCommandService
{
protected:
  // 存储 AT 指令与其回调函数的映射
  static std::map<String, std::function<void(String, String)>> commandCallbacks;

  // 用于存储接收到的数据
  String rxBuffer;

  // 当接收到未知指令时的默认错误响应
  String defaultErrorResponse = "ERROR: Invalid command\r\n";

  // 解析和处理指令的辅助函数
  void processCommand(const String &command);

  // 去除字符串前后空白字符的辅助函数
  String trim(const String &str);

  // 处理 ATH 命令，获取所有注册的命令列表的辅助函数
  void getCommandList();

  // 处理 ATR 命令，重置设备
  void resetDevice();

  // 处理 ATE 命令，用于控制回显
  void handleATECommand(const String &param);

  // 处理 ATI 命令，用于控制回显
  void showInfo();
  // 处理 ATLF 命令，用于 List all files in LittleFS
  void listFiles(const String &param);

  // 处理 ATRF 命令，用于获取文件信息
  void readFile(const String &param);

  // 处理 ATDF 命令，用于删除文件信息
  void deleteFile(const String &param);

  // 处理 ATWF 命令，用于写入文件信息
  void writeFile(const String &param); // under construction

  // 注册 ATOTA 命令，用于启动 ATOTA 服务
  void OTAServer(const String &param);

#ifdef USE_WEB_OTA_SERVER
  // 处理 ATWOTA 命令，start web ota server
  void startWebOTAServer(const String &param);
#endif
public:
  // 构造函数
  ATCommandService();

  // 记录是否启用了回显功能
  bool echoEnabled = true; // 默认为开启回显

  // 注册 AT 指令并绑定回调函数
  void registerCommand(const String &command, std::function<void(String, String)> callback);

  // 处理接收到的数据（从外部调用，例如UART接收的数据）
  void handleIncomingData(const String &data);

  // 发送响应的虚函数（在子类中实现，比如UART、BLE等）
  virtual void sendResponse(const String &response) = 0;

  // 发送回显的虚函数（在子类中实现，比如UART、BLE等）
  virtual void sendEchoCommand(const String &response) = 0;

  // // 设置自定义的错误响应
  // void setDefaultErrorResponse(const String &response);
};

class UARTATCommandService : public ATCommandService
{
private:
  TaskHandle_t taskHandle = NULL; // FreeRTOS 任务句柄

  // FreeRTOS 任务函数，用于处理 UART 数据
  static void taskFunction(void *pvParameters);

public:
  UARTATCommandService();

  // 启动 AT 命令任务
  void startTask(int taskPriority = 1, int stackSize = 4096);

  // 停止 AT 命令任务
  void stopTask();

  // 实现 sendResponse 函数，用于通过 UART 发送响应
  void sendResponse(const String &response) override;

  // 实现 sendEchoCommand 函数，用于通过 UART 回显命令
  void sendEchoCommand(const String &response) override;
};

#ifdef USE_NIMBLE
class BLEATCommandService : public ATCommandService
{
private:
  TaskHandle_t taskHandle = NULL; // FreeRTOS 任务句柄

  // FreeRTOS 任务函数，用于处理 UART 数据
  static void taskFunction(void *pvParameters);

public:
  BLEATCommandService(); // Declare the constructor
  // 启动 AT 命令任务
  void startTask(int taskPriority = 1, int stackSize = 4096);

  // 停止 AT 命令任务
  void stopTask();

  // 实现 sendResponse 函数，用于通过 UART 发送响应
  void sendResponse(const String &response) override;

  // 实现 sendEchoCommand 函数，用于通过 UART 回显命令
  void sendEchoCommand(const String &response) override;
};
#endif
#endif // AT_COMMAND_SERVICE_H
#endif
