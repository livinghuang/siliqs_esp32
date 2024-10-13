#pragma once
#include "bsp.h"

#ifdef USE_NIMBLE
#include "siliqs_heltec_esp32.h"
#include "NimBLEDevice.h"
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
  String toFindDeviceName = "";
  String toFindDeviceAddress = "";
  NimBLEAdvertisedDevice foundDevice;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;

  void init();
  void stop();
  void sendData(const char *data);
  void sendData(String data);
  void setReceivedData(const String &data); // setter 方法
  String getReceivedData();                 // getter 方法

  void scanDevices(int scanTime);                 // 扫描设备
  void task(void *pvParameters);                  // FreeRTOS 任务函数
  static void bleTaskWrapper(void *pvParameters); // FreeRTOS 任务包装

  std::vector<NimBLEAdvertisedDevice> discoveredDevices; // 声明一个设备列表
  void printDiscoveredDevices();
  void startAdvertising(); // 启动广播
  // 声明 processQueue 函数
  void processQueue();

private:
  QueueHandle_t bleQueue; // FreeRTOS队列
  bool scanActive = false;
  NimBLEAdvertising *pAdvertising;
  NimBLEService *pService;                 // 服务
  NimBLECharacteristic *pTxCharacteristic; // TX 特性
  NimBLECharacteristic *pRxCharacteristic; // RX 特性

  String receivedData; // 用于存储接收到的数据
};

extern SQNimBLEService nimbleService; // 创建 NimBLE 服务的实例
extern NimBLEScan *pBLEScan;

#endif