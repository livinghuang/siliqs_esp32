#pragma once
#include "bsp.h"

#ifdef USE_NIMBLE
#include "siliqs_esp32.h"
#include "nimble_src/NimBLEDevice.h"

#include <vector>
#include <freertos/queue.h>
// 定义缓冲队列大小
#define BLE_QUEUE_SIZE 10
#define BLE_MESSAGE_MAX_SIZE 192

#define SQ_SERVICE_UUID "83940000-5273-9374-2109-847320948571"
#define SQ_CHARACTERISTIC_UUID_TX "83940001-5273-9374-2109-847320948571"
#define SQ_CHARACTERISTIC_UUID_RX "83940002-5273-9374-2109-847320948571"

class SQNimBLEService
{
public:
  SQNimBLEService();
  ~SQNimBLEService(); // 析构函数

  NimBLEAdvertisedDevice foundDevice;
  bool deviceFoundWhenScanning = false;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  String toFindDeviceName = "";
  String toFindDeviceAddress = "";
  void init();
  void stop();
  void sendData(const char *data);
  void sendData(String data);
  void setReceivedData(const String &data); // setter 方法
  String getReceivedData();                 // getter 方法

  void startScanDevices(int scanTime);                              // 开始扫描
  void startScanDevices(int scanTime, String name, String address); // 开始扫描
  void stopScanDevices();                                           // 停止扫描
  void rescanDevices(int scanTime);                                 // 重新扫描
  void rescanDevices(int scanTime, String name, String address);    // 重新扫描
  void task(void *pvParameters);                                    // FreeRTOS 任务函数
  static void bleTaskWrapper(void *pvParameters);                   // FreeRTOS 任务包装

  std::vector<NimBLEAdvertisedDevice> discoveredDevices; // 声明一个设备列表
  void printDiscoveredDevices();
  void startAdvertising(); // 启动广播
  // 声明 processQueue 函数
  void processQueue();

private:
  QueueHandle_t bleQueue; // FreeRTOS队列
  bool scanActive = false;
  int scanTime = 0;

  NimBLEAdvertising *pAdvertising;
  NimBLEService *pService;                 // 服务
  NimBLECharacteristic *pTxCharacteristic; // TX 特性
  NimBLECharacteristic *pRxCharacteristic; // RX 特性

  String receivedData; // 用于存储接收到的数据
};

extern SQNimBLEService nimbleService; // 创建 NimBLE 服务的实例
extern NimBLEScan *pBLEScan;

#endif