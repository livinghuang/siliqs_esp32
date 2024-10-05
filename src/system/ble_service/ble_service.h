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

class SQ_BLEServiceClass
{
public:
  void init(unsigned long timeout);
  void stop();
  void sendData(const char *data);
  void task(void *pvParameters);                  // FreeRTOS 任务函数
  static void bleTaskWrapper(void *pvParameters); // FreeRTOS 任务包装
  bool deviceConnected = false;
  bool oldDeviceConnected = false;

private:
  BLEServer *pServer = nullptr;
  BLECharacteristic *pTxCharacteristic;

  unsigned long waiting_connect_timeout = 30000; // 默认超时时间
  unsigned long startTime;

  const char *SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  const char *CHARACTERISTIC_UUID_TX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
  const char *CHARACTERISTIC_UUID_RX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
};

extern SQ_BLEServiceClass SQ_BLEService;

#endif
#endif // USE_BLE 结束