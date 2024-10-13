#include "bsp.h"
#include "siliqs_heltec_esp32.h"

void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO);
  // 初始化串口通信
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10); // 等待串口连接
  }

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
  nimbleService.toFindDeviceName = "HK99";
  Serial.println("NimBLE 服务初始化完成，开始扫描...");
}

void loop()
{
  String target_address = "00:01:02:03:04:05";

  static uint32_t time = millis();
  if (millis() - time > 5000)
  {
    time = millis();
    nimbleService.printDiscoveredDevices();
  }
  // Check if a specific device was found
  if (!nimbleService.foundDevice.getName().empty())
  {
    Serial.printf("Found Device by Name: %s \n", nimbleService.foundDevice.toString().c_str());

    if (String(nimbleService.foundDevice.getAddress().toString().c_str()) == target_address)
    {
      Serial.printf("Match Device Address: %s \n", nimbleService.foundDevice.getAddress().toString().c_str());
      Serial.printf("Device manufacturer data: %s \n", nimbleService.foundDevice.getManufacturerData().c_str());
    }
    else
    {
      Serial.println("Device address not match, skipped.");
    }

    // Reset foundDevice to an empty state after processing
    nimbleService.foundDevice = NimBLEAdvertisedDevice();
  }
}