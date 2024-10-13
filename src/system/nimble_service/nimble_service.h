#pragma once
#include "bsp.h"

#ifdef USE_NIMBLE
#include "siliqs_heltec_esp32.h"
#include "NimBLEDevice.h"
#include <vector>
class SQNimBLEService
{
public:
  SQNimBLEService();
  ~SQNimBLEService(); // 析构函数
  String toFindDeviceName = "";
  String toFindDeviceAddress = "";
  NimBLEAdvertisedDevice foundDevice;

  void init();
  void task(void *pvParameters);                  // FreeRTOS 任务函数
  static void bleTaskWrapper(void *pvParameters); // FreeRTOS 任务包装

  std::vector<NimBLEAdvertisedDevice> discoveredDevices; // 声明一个设备列表
  void printDiscoveredDevices();

private:
  NimBLEAdvertising *pAdvertising;
};
extern SQNimBLEService nimbleService; // 创建 NimBLE 服务的实例
extern NimBLEScan *pBLEScan;

#endif