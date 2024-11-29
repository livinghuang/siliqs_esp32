#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE

#include "at_command_service.h"

std::map<String, std::function<void(String, String)>> ATCommandService::commandCallbacks;

// ATCommandService 构造函数，初始化默认命令
ATCommandService::ATCommandService()
{
  // 注册默认的 "AT" 命令
  registerCommand("AT", [this](String cmd, String param)
                  { this->sendResponse("OK\r\n"); });
  // 注册 "ATE" 命令，用于控制回显
  registerCommand("ATE", [this](String cmd, String param)
                  { this->handleATECommand(param); });
  // 注册 "ATH" 命令，列出所有命令
  registerCommand("ATH", [this](String cmd, String param)
                  { this->getCommandList(); });
  // 注册 "ATR" 命令，模拟设备重启
  registerCommand("ATR", [this](String cmd, String param)
                  { this->resetDevice(); });
  // 注册 "ATI" 命令，显示设备信息
  registerCommand("ATI", [this](String cmd, String param)
                  { this->showInfo(); });
  // 注册 "ATLF 命令，List all files in LittleFS
  registerCommand("ATLF", [this](String cmd, String param)
                  { this->listFiles(param); });
  // 注册 ATRF 命令，用于获取文件信息
  registerCommand("ATRF", [this](String cmd, String param)
                  { this->readFile(param); });
  // 注册 ATDF 命令，用于删除文件信息
  registerCommand("ATDF", [this](String cmd, String param)
                  { this->deleteFile(param); });
  // 注册 ATWF 命令，用于写入文件信息
  registerCommand("ATWF", [this](String cmd, String param)
                  { this->writeFile(param); });
#ifdef USE_WEB_OTA_SERVER
  // 注册 ATWOTA 命令，用于启动 WebOTA 服务
  registerCommand("ATWOTA", [this](String cmd, String param)
                  { this->startWebOTAServer(param); });
#endif
}

// 去除字符串前后空白字符
String ATCommandService::trim(const String &str)
{
  int first = 0;
  while (first < str.length() && str[first] == ' ')
  {
    first++;
  }

  if (first == str.length())
  {
    return "";
  }

  int last = str.length() - 1;
  while (last >= 0 && str[last] == ' ')
  {
    last--;
  }

  return str.substring(first, last + 1);
}

// 解析和处理指令的函数
void ATCommandService::processCommand(const String &command)
{
  String trimmedCommand = trim(command);

  // 查找等号 '='，用于分隔命令和参数
  int equalPos = trimmedCommand.indexOf('=');
  String cmdOnly;
  String param;

  if (equalPos != -1)
  {
    // 提取命令（等号左边的部分）
    cmdOnly = trimmedCommand.substring(0, equalPos);
    // 提取参数（等号右边的部分）
    param = trimmedCommand.substring(equalPos + 1);
  }
  else
  {
    // 没有参数，整个字符串就是命令
    cmdOnly = trimmedCommand;
    param = "";
  }

  // 查找已注册的 AT 指令
  auto it = commandCallbacks.find(cmdOnly);
  if (it != commandCallbacks.end())
  {
    // 调用注册的回调函数，并传递命令和参数
    it->second(cmdOnly, param);
  }
  else
  {
    // 未找到匹配指令，发送默认错误响应
    sendResponse(defaultErrorResponse);
  }
}

// 注册 AT 指令并绑定回调函数
void ATCommandService::registerCommand(const String &command, std::function<void(String, String)> callback)
{
  // 检查命令是否已经注册
  if (commandCallbacks.find(command) != commandCallbacks.end())
  {
    // 如果命令已存在，可以发送一个警告或处理
    console.log(sqWARNING, "Command already registered: " + command);
    return; // 或者根据需求选择返回错误或覆盖旧的回调函数
  }
  commandCallbacks[command] = callback;
  console.log(sqINFO, "Registered command: " + command);
}

// 处理接收到的数据
void ATCommandService::handleIncomingData(const String &data)
{
  // 将数据追加到接收缓冲区
  rxBuffer += data;

  // 查找指令结束符（如 "\r\n"）
  int pos;
  while ((pos = rxBuffer.indexOf("\r\n")) != -1)
  {
    // 提取完整的指令
    String command = rxBuffer.substring(0, pos);

    console.log(sqDEBUG, "Received command: " + command);
    // Echo back
    if (echoEnabled)
    {
      console.log(sqDEBUG, "Echoing back: " + command);
      sendEchoCommand(command);
    }

    // 处理指令
    processCommand(command);
    // 从缓冲区中移除已处理的指令
    rxBuffer.remove(0, pos + 2);
  }
}
#endif