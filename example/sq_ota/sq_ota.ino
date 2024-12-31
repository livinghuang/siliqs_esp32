#include "siliqs_esp32.h"
#include "system/ota_service/ota_service.h"
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
  BLEatService.startTask();
#endif
  // 启动后台任务处理 AT 命令
  UARTatService.startTask();
  UARTatService.echoEnabled = false;
}

void loop()
{
}