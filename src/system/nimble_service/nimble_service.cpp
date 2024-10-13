#include "nimble_service.h"

#ifdef USE_NIMBLE

// 自定义的设备回调类
class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
  void onResult(NimBLEAdvertisedDevice *advertisedDevice)
  {
    // console.log(sqINFO, "Found Device: %s \n", advertisedDevice->toString().c_str());
    // 将扫描到的设备存储到列表中
    nimbleService.discoveredDevices.push_back(*advertisedDevice);
    if (nimbleService.toFindDeviceName == String(advertisedDevice->getName().c_str()))
    {

      nimbleService.foundDevice = *advertisedDevice;
      console.log(sqINFO, "Found Device By Name: %s \n", nimbleService.foundDevice.toString().c_str());
    }
  }
};

SQNimBLEService::SQNimBLEService()
{
  // 构造函数
}

SQNimBLEService::~SQNimBLEService()
{
  if (pBLEScan->isScanning())
  {
    pBLEScan->stop(); // 停止扫描
  }
  NimBLEDevice::deinit(); // 释放 NimBLE 资源
}

void SQNimBLEService::init()
{
  Serial.println("初始化 NimBLE...");

  NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE);
  NimBLEDevice::setScanDuplicateCacheSize(200);
  NimBLEDevice::init("");

  pBLEScan = NimBLEDevice::getScan(); // 创建扫描对象
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
  pBLEScan->setActiveScan(true); // 启用主动扫描
  pBLEScan->setInterval(97);     // 扫描间隔，单位：毫秒
  pBLEScan->setWindow(37);       // 扫描窗口，单位：毫秒
  pBLEScan->setMaxResults(0);    // 不存储扫描结果，只通过回调处理
}

void SQNimBLEService::printDiscoveredDevices()
{
  if (discoveredDevices.empty())
  {
    Serial.println("No devices discovered yet.");
    return;
  }

  Serial.printf("Discovered %d devices:\n", discoveredDevices.size());

  for (size_t i = 0; i < discoveredDevices.size(); i++)
  {
    NimBLEAdvertisedDevice &device = discoveredDevices[i];

    // Print device name (if available), address, and RSSI
    Serial.printf("Device %d:\n", i + 1);
    Serial.printf("  Name: %s\n", device.getName().empty() ? "Unnamed" : device.getName().c_str());
    Serial.printf("  Address: %s\n", device.getAddress().toString().c_str());
    Serial.printf("  RSSI: %d\n", device.getRSSI());
  }
}

void SQNimBLEService::bleTaskWrapper(void *pvParameters)
{
  SQNimBLEService *instance = static_cast<SQNimBLEService *>(pvParameters);
  instance->task(pvParameters); // 调用成员函数 task()
}

void SQNimBLEService::task(void *pvParameters)
{
  while (1)
  {
    // 如果扫描意外停止，重新启动扫描
    if (!pBLEScan->isScanning())
    {
      pBLEScan->start(0, nullptr, false); // 无限时长扫描
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // 每 100 毫秒检查一次
  }
}
SQNimBLEService nimbleService; // 创建 NimBLE 服务的实例
NimBLEScan *pBLEScan = nullptr;

#endif // USE_NIMBLE