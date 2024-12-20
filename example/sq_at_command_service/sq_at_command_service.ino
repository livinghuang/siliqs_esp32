#include "bsp.h"
#include "siliqs_esp32.h"

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */

#ifdef USE_NIMBLE
// 创建 BLEATCommandService 实例
BLEATCommandService BLEatService;
#endif
// 创建 UARTATCommandService 实例，使用 Serial 作为通信接口
UARTATCommandService UARTatService;

void setup()
{
  siliqs_esp32_setup(SQ_INFO);
  Serial.println("AT Command Service is starting...");
#ifdef USE_NIMBLE
  // console.log(sqINFO, "Start BLE service");
  // // 初始化 BLE 服务
  // SQ_BLEService.init(30000, "SQ-"); // your could name the prefix of your device, it should looks like "SQ-xxxx"
  // // 创建 BLE 服务的 FreeRTOS 任务
  // xTaskCreate(SQ_BLEServiceClass::bleTaskWrapper, "bleTask", 4096, NULL, 1, NULL);
  // 注册带参数的 "AT+TEMP" 命令
  // BLEatService.registerCommand("AT+TEMP", [](String cmd, String param)
  //
  //  {
  //       if (param.length() > 0)
  //       {
  //           // 解析参数为浮点数，并发送回响应
  //           float temp = param.toFloat();
  //           BLEatService.sendResponse("Temperature set to " + String(temp) + " degrees.\r\n");
  //       }
  //       else
  //       {
  //         BLEatService.sendResponse("ERROR: Missing parameter for AT+TEMP\r\n");
  //       } });
  // 启动后台任务处理 AT 命令
  BLEatService.startTask();
#endif
  // 注册带参数的 "AT+TEMP" 命令
  UARTatService.registerCommand("AT+TEMP", [](String cmd, String param)
                                {
        if (param.length() > 0)
        {
            // 解析参数为浮点数，并发送回响应
            float temp = param.toFloat();
            UARTatService.sendResponse("Temperature set to " + String(temp) + " degrees.\r\n");
        }
        else
        {
            UARTatService.sendResponse("ERROR: Missing parameter for AT+TEMP\r\n");
        } });
  // 启动后台任务处理 AT 命令
  UARTatService.startTask();
}

void loop()
{
  // 主程序可以继续处理其他任务，AT 命令处理在后台运行
  // 例如，处理其他传感器输入或逻辑处理
  // 这里可以放置其他的定时操作等
}