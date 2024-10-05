#include "bsp.h"

#ifdef USE_BLE // 仅在启用 USE_BLE 时编译

#include "ble_service.h"

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    SQ_BLEService.deviceConnected = true; // 使用新的实例 SQ_BLEService
    console.log(sqINFO, "Client connected!");
  }

  void onDisconnect(BLEServer *pServer)
  {
    SQ_BLEService.deviceConnected = false; // 使用新的实例 SQ_BLEService
    console.log(sqINFO, "Client disconnected!");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    console.log(sqINFO, "Received Value: " + pCharacteristic->getValue()); // 将 std::string 转换为 String 输出
  }
};

void SQ_BLEServiceClass::init(unsigned long timeout, String namePrefix)
{
  console.log(sqINFO, "Initializing BLE...");

  // 初始化 BLE 设备
  // 获取 ESP32 的 MAC 地址
  uint64_t macAddress = ESP.getEfuseMac();

  // // 提取 MAC 地址的最后 2 字节并转换为 4 位十六进制数字
  uint16_t macShort = (uint16_t)(macAddress >> 32); // 高 16 位
  // 生成设备名称：格式为 "xxx-1234"
  String deviceName = namePrefix + String(macShort, HEX);
  deviceName.toUpperCase();
  BLEDevice::init(deviceName);

  // 创建 BLE 服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建 BLE 服务
  BLEService *pService = pServer->createService(SERVICE_UUID); // 使用 BLEService 创建服务

  // 创建发送特性 (TX)
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  // 创建接收特性 (RX)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // 启动服务
  pService->start();

  // 开始广播
  pServer->getAdvertising()->start();
  console.log(sqINFO, "Waiting for a client to connect...");

  startTime = millis();
  waiting_connect_timeout = timeout;
  vTaskDelay(1);
}

void SQ_BLEServiceClass::stop()
{
  // 停止 BLE 服务
  pServer->disconnect(pServer->getConnId());
  pServer->getAdvertising()->stop();
  BLEDevice::deinit();
  console.log(sqINFO, "BLE service stopped.");
}

void SQ_BLEServiceClass::sendData(const char *data)
{
  if (deviceConnected)
  {
    pTxCharacteristic->setValue((uint8_t *)data, strlen(data));
    pTxCharacteristic->notify();
    vTaskDelay(10); // 避免堆栈拥塞
    console.log(sqINFO, "Data sent to client.");
  }
}

void SQ_BLEServiceClass::bleTaskWrapper(void *pvParameters)
{
  SQ_BLEService.task(pvParameters); // 使用 SQ_BLEService 实例
}

void SQ_BLEServiceClass::task(void *pvParameters)
{
  while (1)
  {
    if (deviceConnected)
    {
      startTime = millis(); // 重置计时器
    }
    else if (millis() - startTime >= waiting_connect_timeout)
    {
      console.log(sqINFO, "Timeout occurred, restarting advertising...");
      pServer->getAdvertising()->start();
      startTime = millis();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // 每 100 毫秒处理一次
  }
}

// 定义 SQ_BLEService 实例
SQ_BLEServiceClass SQ_BLEService;

#endif // USE_BLE 宏结束