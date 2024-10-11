#include "bsp.h"

#ifdef USE_BLE // 仅在启用 USE_BLE 时编译

#include "ble_service.h"

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    SQ_BLEService.deviceConnected = true;
    console.log(sqINFO, "Client connected!");
  }

  void onDisconnect(BLEServer *pServer)
  {
    SQ_BLEService.deviceConnected = false;
    console.log(sqINFO, "Client disconnected!");
    // 重新启动广告，以便客户端可以再次发现该服务
    pServer->getAdvertising()->start();
    console.log(sqINFO, "Restarting advertising...");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    // 获取接收的数据并将其存储到 SQ_BLEService 的成员变量中
    // 使用 setReceivedData 方法存储接收到的数据
    SQ_BLEService.setReceivedData(pCharacteristic->getValue().c_str());
    console.log(sqINFO, "Received Value: " + pCharacteristic->getValue()); // 将 std::string 转换为 String 输出
  }
};

void SQ_BLEServiceClass::init(unsigned long timeout, String namePrefix)
{
  console.log(sqINFO, "Initializing BLE...");

  // 初始化 BLE 设备
  // 获取 ESP32 的 MAC 地址
  uint64_t macAddress = ESP.getEfuseMac();

  // 提取 MAC 地址的最后 2 字节并转换为 4 位十六进制数字
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

  // 创建发送特性 (TX)，并设置为通知特性
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);

  // 添加 BLE2902 描述符，允许客户端启用或禁用通知
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

  // 创建消息队列
  bleQueue = xQueueCreate(BLE_QUEUE_SIZE, BLE_MESSAGE_MAX_SIZE);

  // 创建处理发送数据的任务
  xTaskCreate(SQ_BLEServiceClass::sendDataTask, "BLESendDataTask", 4096, this, 1, NULL);

  startTime = millis();
  waiting_connect_timeout = timeout;
  vTaskDelay(1);
}

void SQ_BLEServiceClass::stop()
{
  // 停止BLE服务
  pServer->disconnect(pServer->getConnId());
  pServer->getAdvertising()->stop();
  BLEDevice::deinit();

  // 删除队列
  if (bleQueue != NULL)
  {
    vQueueDelete(bleQueue);
    bleQueue = NULL;
  }

  console.log(sqINFO, "BLE service stopped.");
}
void SQ_BLEServiceClass::sendDataTask(void *pvParameters)
{
  SQ_BLEServiceClass *bleService = (SQ_BLEServiceClass *)pvParameters;
  char dataBuffer[BLE_MESSAGE_MAX_SIZE];

  while (1)
  {
    // 从队列中接收数据
    if (xQueueReceive(bleService->bleQueue, &dataBuffer, portMAX_DELAY) == pdPASS)
    {
      // 发送数据给客户端
      if (bleService->deviceConnected)
      {
        bleService->pTxCharacteristic->setValue((uint8_t *)dataBuffer, strlen(dataBuffer));
        bleService->pTxCharacteristic->notify();
        vTaskDelay(10 / portTICK_PERIOD_MS); // 延迟以避免BLE堆栈过载
        console.log(sqINFO, "Data sent from queue to client.");
      }
      else
      {
        console.log(sqERROR, "No device connected, dropping data.");
      }
    }
  }
}
void SQ_BLEServiceClass::sendData(const char *data)
{
  if (deviceConnected)
  {
    // 检查数据长度
    size_t dataLen = strlen(data);
    if (dataLen > BLE_MESSAGE_MAX_SIZE)
    {
      console.log(sqERROR, "Data is too large to send!");
      return;
    }

    // 将数据放入队列
    if (xQueueSend(bleQueue, data, pdMS_TO_TICKS(100)) != pdPASS)
    {
      console.log(sqERROR, "Queue is full, data dropped!");
    }
    else
    {
      console.log(sqINFO, "Data queued successfully.");
    }
  }
}

void SQ_BLEServiceClass::sendData(String data)
{
  sendData(data.c_str());
}

void SQ_BLEServiceClass::setReceivedData(const String &data)
{
  receivedData = data;
}
// 获取接收到的数据
String SQ_BLEServiceClass::getReceivedData()
{
  return receivedData;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
private:
  SQ_BLEServiceClass &bleService; // 引用 SQ_BLEServiceClass 实例

public:
  // 构造函数，传递 SQ_BLEServiceClass 的引用
  MyAdvertisedDeviceCallbacks(SQ_BLEServiceClass &service) : bleService(service) {}

  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // 获取当前设备的地址
    BLEAddress address = advertisedDevice.getAddress();

    // 查找设备是否已经存在于设备列表中
    auto it = std::find_if(bleService.discoveredDevices.begin(), bleService.discoveredDevices.end(),
                           [&address](const BLEAdvertisedDevice &device)
                           {
                             return const_cast<BLEAdvertisedDevice &>(device).getAddress() == address;
                           });

    // 如果设备已存在，先删除旧设备
    if (it != bleService.discoveredDevices.end())
    {
      console.log(sqINFO, "Device already discovered, updating: " + address.toString());
      bleService.discoveredDevices.erase(it); // 删除旧设备
    }

    // 无论设备是否存在，都插入新设备以更新状态
    bleService.discoveredDevices.push_back(advertisedDevice); // 插入新设备
    console.log(sqINFO, "Device updated: " + address.toString());

    // 打印设备的详细信息
    if (advertisedDevice.haveName())
    {
      console.log(sqINFO, "Device name: " + advertisedDevice.getName());
    }
    if (advertisedDevice.haveRSSI())
    {
      console.log(sqINFO, "RSSI: " + String(advertisedDevice.getRSSI()));
    }
  }
};

void SQ_BLEServiceClass::scanDevices(int scanTime)
{
  console.log(sqINFO, "Starting BLE scan...");

  BLEScan *pBLEScan = BLEDevice::getScan();                                       // 创建扫描实例
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(*this)); // 传递 SQ_BLEService 实例
  pBLEScan->setActiveScan(true);                                                  // 开启主动扫描模式
  pBLEScan->start(scanTime, false);                                               // 扫描 `scanTime` 秒

  console.log(sqINFO, "Scan completed.");
}

void SQ_BLEServiceClass::bleTaskWrapper(void *pvParameters)
{
  SQ_BLEService.task(pvParameters); // 使用 SQ_BLEService 实例
}

// if second is 0, scan all the time
// if second is not 0, scan `second` seconds
void SQ_BLEServiceClass::startDiscovery(int seconds)
{
  console.log(sqINFO, "Manually starting BLE scan...");

  doDiscovery = true;
  scanTime = seconds;
}

void SQ_BLEServiceClass::stopDiscovery()
{
  console.log(sqINFO, "Manually stopping BLE scan...");
  doDiscovery = false;
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

    // 设备扫描逻辑
    if (doDiscovery)
    {
      if (scanTime == 0)
      {
        console.log(sqINFO, "Starting BLE continuously scan...");
        scanDevices(5); // 扫描 5 秒钟
        // then restart the scan till doDiscovery is set to false
      }
      else
      {
        doDiscovery = false;
        console.log(sqINFO, "Starting BLE scan in " + String(scanTime) + " seconds...");
        scanDevices(scanTime);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // 每 100 毫秒处理一次
  }
}

// 定义 SQ_BLEService 实例
SQ_BLEServiceClass SQ_BLEService;

#endif // USE_BLE 宏结束