#include "bsp.h"
#include "siliqs_heltec_esp32.h"

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */

// 创建 UARTATCommandService 实例，使用 Serial 作为通信接口
UARTATCommandService atService(Serial);

void setup()
{
  // 初始化 UART，波特率为 115200
  Serial.begin(115200);
  Serial.println("AT Command Service is starting...");

  // 注册带参数的 "AT+TEMP" 命令
  atService.registerCommand("AT+TEMP", [](String cmd, String param)
                            {
        if (param.length() > 0)
        {
            // 解析参数为浮点数，并发送回响应
            float temp = param.toFloat();
            atService.sendResponse("Temperature set to " + String(temp) + " degrees.\r\n");
        }
        else
        {
            atService.sendResponse("ERROR: Missing parameter for AT+TEMP\r\n");
        } });

  // 注册 "AT+RESET" 命令，模拟设备重启
  atService.registerCommand("AT+RESET", [](String cmd, String param)
                            {
                              atService.sendResponse("Device is resetting...\r\n");
                              delay(1000);
                              ESP.restart(); // 使用 ESP32 的重启功能
                            });

  // 设置默认错误响应
  atService.setDefaultErrorResponse("ERROR: Invalid command\r\n");

  // 启动后台任务处理 AT 命令
  atService.startTask();
}

void loop()
{
  // 主程序可以继续处理其他任务，AT 命令处理在后台运行
  // 例如，处理其他传感器输入或逻辑处理
  // 这里可以放置其他的定时操作等
}