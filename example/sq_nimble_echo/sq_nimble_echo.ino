#include "bsp.h"
#include "siliqs_heltec_esp32.h"
void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO);

  Serial.println("初始化 NimBLE 服务...");

  // 初始化 NimBLE 服务
  nimbleService.init();

  // 创建一个 FreeRTOS 任务来处理 BLE 扫描
  xTaskCreate(
      SQNimBLEService::bleTaskWrapper, // 任务函数包装器
      "NimBLE Scan Task",              // 任务名称
      4096,                            // 堆栈大小（字节）
      &nimbleService,                  // 传递给任务的参数（NimBLE 服务实例）
      1,                               // 任务优先级
      NULL                             // 任务句柄（可以为 NULL）
  );
  Serial.println("NimBLE 服务初始化完成");
}

void loop()
{
  if (nimbleService.deviceConnected)
  {
    String data = nimbleService.getReceivedData();
    nimbleService.setReceivedData("");
    if (data.length() > 0)
    {
      Serial.println(data);
      nimbleService.sendData(data);
    }
  }
  delay(10);
}