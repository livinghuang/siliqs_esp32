#pragma once
#include "bsp.h"

#ifdef USE_BLE // 仅在启用 USE_BLE 时编译

#ifndef _BLE_SERVICE_H_
#define _BLE_SERVICE_H_

#include "siliqs_heltec_esp32.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <algorithm>

#include <freertos/queue.h>

// 定义缓冲队列大小
#define BLE_QUEUE_SIZE 10
#define BLE_MESSAGE_MAX_SIZE 192

#define SQ_SERVICE_UUID "83940000-5273-9374-2109-847320948571"
#define SQ_CHARACTERISTIC_UUID_TX "83940001-5273-9374-2109-847320948571"
#define SQ_CHARACTERISTIC_UUID_RX "83940002-5273-9374-2109-847320948571"

class SQ_BLEServiceClass
{
public:
  void init(unsigned long timeout, String namePrefix = "BLE-");
  void stop();
  void sendData(const char *data);
  void sendData(String data);
  void setReceivedData(const String &data); // 新增一个 setter 方法
  String getReceivedData();
  void scanDevices(int scanTime);                 // 新增方法，设置扫描时间
  void task(void *pvParameters);                  // FreeRTOS 任务函数
  static void bleTaskWrapper(void *pvParameters); // FreeRTOS 任务包装
  void startDiscovery(int seconds);               // 手动启动设备扫描
  void stopDiscovery();                           // 手动停止设备扫描

  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  std::vector<BLEAdvertisedDevice> discoveredDevices; // 声明一个设备列表
  static void sendDataTask(void *pvParameters);       // 将任务函数声明为静态成员函数

private:
  QueueHandle_t bleQueue; // FreeRTOS队列
  BLEServer *pServer = nullptr;
  BLECharacteristic *pTxCharacteristic;

  unsigned long waiting_connect_timeout = 30000; // 默认超时时间
  unsigned long startTime;

  const char *SERVICE_UUID = SQ_SERVICE_UUID;
  const char *CHARACTERISTIC_UUID_TX = SQ_CHARACTERISTIC_UUID_TX;
  const char *CHARACTERISTIC_UUID_RX = SQ_CHARACTERISTIC_UUID_RX;

  bool doDiscovery = false;
  int scanTime = 5; // 扫描时间
  String receivedData = "";
};

extern SQ_BLEServiceClass SQ_BLEService;

#endif
#endif // USE_BLE 结束