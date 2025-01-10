#include "nimble_service.h"

#ifdef USE_NIMBLE

// 自定义服务器回调类，用于处理连接/断开连接事件
class MyServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer) override
  {
    nimbleService.deviceConnected = true;
    console.log(sqINFO, "设备已连接.");
  }

  void onDisconnect(NimBLEServer *pServer) override
  {
    nimbleService.deviceConnected = false;
    console.log(sqINFO, "设备已断开.");
    nimbleService.startAdvertising(); // 重新启动广播
  }
};

// 自定义的设备回调类
class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
  void onResult(NimBLEAdvertisedDevice *advertisedDevice) override
  {
    bool deviceExists = false;
    // console.log(sqINFO, "found device: %s", advertisedDevice->toString().c_str());
    if ((nimbleService.toFindDeviceAddress != "") &&
        (nimbleService.toFindDeviceAddress == String(advertisedDevice->getAddress().toString().c_str())))
    {
      nimbleService.deviceFoundWhenScanning = true;
      nimbleService.foundDevice = *advertisedDevice;
      console.log(sqINFO, "Found Device By Address: %s \n", nimbleService.foundDevice.toString().c_str());
    }
    if ((nimbleService.toFindDeviceName != "") && (nimbleService.toFindDeviceName == String(advertisedDevice->getName().c_str())))
    {
      nimbleService.deviceFoundWhenScanning = true;
      nimbleService.foundDevice = *advertisedDevice;
      console.log(sqINFO, "Found Device By Name: %s \n", nimbleService.foundDevice.toString().c_str());
    }
    // 检查设备是否已经存在
    for (const auto &device : nimbleService.discoveredDevices)
    {
      if (const_cast<NimBLEAdvertisedDevice &>(device).getAddress().equals(advertisedDevice->getAddress())) // 问题在这里
      {
        deviceExists = true;
        break;
      }
    }

    // 如果设备不存在，则添加到列表中
    if (!deviceExists)
    {
      nimbleService.discoveredDevices.push_back(*advertisedDevice);
    }
  }
};

// 自定义的 RX 特性回调类
class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic) override
  {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0)
    {
      console.log(sqINFO, "Received Data: " + String(value.c_str()));
      nimbleService.setReceivedData(String(value.c_str()));
    }
  }
};

void SQNimBLEService::startAdvertising()
{
  console.log(sqINFO, "开始广播...");

  pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SQ_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // 设置最低首选连接间隔
  pAdvertising->setMaxPreferred(0x12); // 设置最大首选连接间隔
  pAdvertising->start();
  console.log(sqINFO, "广播已启动.");
}

// 构造函数
SQNimBLEService::SQNimBLEService()
    : bleQueue(nullptr), scanActive(false), pAdvertising(nullptr), pService(nullptr), pTxCharacteristic(nullptr), pRxCharacteristic(nullptr)
{
  // 创建队列，用于存储要发送的消息
  bleQueue = xQueueCreate(BLE_QUEUE_SIZE, BLE_MESSAGE_MAX_SIZE * sizeof(char));
  if (bleQueue == nullptr)
  {
    console.log(sqERROR, "无法创建 BLE 消息队列");
    return;
  }
}

// 析构函数
SQNimBLEService::~SQNimBLEService()
{
  if (pBLEScan && pBLEScan->isScanning())
  {
    pBLEScan->stop();
  }
  NimBLEDevice::deinit();
  if (bleQueue != nullptr)
  {
    vQueueDelete(bleQueue);
  }
}

void SQNimBLEService::init()
{
  console.log(sqINFO, "初始化 NimBLE 服务...");

  char deviceName[13];
  long unsigned int espmac = ESP.getEfuseMac() >> 24;
  snprintf(deviceName, 13, "SQ-%06lX", espmac);

  NimBLEDevice::init(deviceName); // 初始化 NimBLE 设备
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // 设置服务器回调

  pService = pServer->createService(SQ_SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(SQ_CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
  pRxCharacteristic = pService->createCharacteristic(SQ_CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks()); // 设置 RX 特性回调

  pService->start();  // 启动服务
  startAdvertising(); // 启动广播
}

// 实现设备扫描
void SQNimBLEService::startScanDevices(int scanTime)
{
  if (!scanActive)
  {
    console.log(sqINFO, "start scanning...");
    pBLEScan = NimBLEDevice::getScan(); // 创建扫描对象
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
    pBLEScan->setActiveScan(true); // 启用主动扫描
    pBLEScan->setInterval(97);     // 扫描间隔，单位：毫秒
    pBLEScan->setWindow(37);       // 扫描窗口，单位：毫秒
    pBLEScan->setMaxResults(0);    // 不存储扫描结果，只通过回调处理
    pBLEScan->start(scanTime, nullptr);
    scanActive = true;
    deviceFoundWhenScanning = false;
  }
  else
  {
    console.log(sqINFO, "scan is already active...");
  }
}

// 实现设备扫描
void SQNimBLEService::startScanDevices(int _scanTime, String _name, String _address)
{
  toFindDeviceName = _name;
  toFindDeviceAddress = _address;
  scanTime = _scanTime;
  if (!scanActive)
  {
    console.log(sqINFO, "start scanning...");
    pBLEScan = NimBLEDevice::getScan(); // 创建扫描对象
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), false);
    pBLEScan->setActiveScan(true); // 启用主动扫描
    pBLEScan->setInterval(97);     // 扫描间隔，单位：毫秒
    pBLEScan->setWindow(37);       // 扫描窗口，单位：毫秒
    pBLEScan->setMaxResults(0);    // 不存储扫描结果，只通过回调处理
    pBLEScan->start(scanTime, nullptr);
    scanActive = true;
    deviceFoundWhenScanning = false;
  }
  else
  {
    console.log(sqINFO, "scan is already active...");
  }
}

// 停止扫描
void SQNimBLEService::stopScanDevices()
{
  if (pBLEScan != nullptr && pBLEScan->isScanning())
  {
    console.log(sqINFO, "stop scanning...");
    toFindDeviceName = "";
    toFindDeviceAddress = "";
    pBLEScan->stop();
    scanActive = false;
  }
}

// 重新扫描
void SQNimBLEService::rescanDevices(int _scanTime, String _toFindDeviceName, String _toFindDeviceAddress)
{
  scanTime = _scanTime;
  toFindDeviceName = _toFindDeviceName;
  toFindDeviceAddress = _toFindDeviceAddress;
  stopScanDevices();
  startScanDevices(scanTime, toFindDeviceName, toFindDeviceAddress);
}
void SQNimBLEService::rescanDevices(int _scanTime)
{
  scanTime = _scanTime;
  stopScanDevices();
  startScanDevices(scanTime);
}

void SQNimBLEService::processQueue()
{
  if (bleQueue == nullptr)
  {
    console.log(sqERROR, "消息队列未初始化");
    return;
  }

  char message[BLE_MESSAGE_MAX_SIZE];

  while (xQueueReceive(bleQueue, message, portMAX_DELAY) == pdPASS)
  {
    // 确保取出的消息是有效的
    size_t messageLength = strlen(message);
    if (messageLength > 0)
    {
      // 从队列中取出消息并发送
      if (pTxCharacteristic != nullptr && deviceConnected)
      {
        pTxCharacteristic->setValue((uint8_t *)message, messageLength);
        pTxCharacteristic->notify();
        console.log(sqINFO, "发送队列中的消息: %s", message);
      }
      else
      {
        console.log(sqERROR, "无法发送消息，没有连接或 TX 特性未初始化");
      }
    }
    else
    {
      console.log(sqERROR, "消息为空，无法发送");
    }

    // 清空消息缓冲区，以避免残留消息影响下次接收
    memset(message, 0, sizeof(message));
  }
}
// 发送消息，将消息放入队列
void SQNimBLEService::sendData(const char *data)
{
  if (bleQueue == nullptr || data == nullptr)
  {
    console.log(sqERROR, "消息队列未初始化或消息为空");
    return;
  }

  // 尝试将消息放入队列，等待 100ms 超时时间
  if (xQueueSendToBack(bleQueue, data, portMAX_DELAY) != pdPASS)
  {
    console.log(sqERROR, "队列已满，无法添加消息");
  }
  else
  {
    console.log(sqINFO, "消息已添加到队列: %s", data);
  }
}

void SQNimBLEService::sendData(String data)
{
  sendData(data.c_str());
}

void SQNimBLEService::setReceivedData(const String &data)
{
  receivedData = data;
}

String SQNimBLEService::getReceivedData()
{
  return receivedData;
}

void SQNimBLEService::printDiscoveredDevices()
{
  if (discoveredDevices.empty())
  {
    Serial.println("No devices discovered yet.");
    return;
  }

  console.log(sqINFO, "Discovered %d devices:\n", discoveredDevices.size());

  for (size_t i = 0; i < discoveredDevices.size(); i++)
  {
    NimBLEAdvertisedDevice &device = discoveredDevices[i];
    Serial.printf("Device %d:\n  Name: %s\n  Address: %s\n  RSSI: %d\n",
                  i + 1, device.getName().empty() ? "Unnamed" : device.getName().c_str(),
                  device.getAddress().toString().c_str(), device.getRSSI());
  }
}
void SQNimBLEService::stop()
{
  console.log(sqINFO, "停止 NimBLE 服务和广播...");

  // 停止广播
  if (pAdvertising != nullptr && pAdvertising->isAdvertising())
  {
    pAdvertising->stop();
    console.log(sqINFO, "广播已停止.");
  }

  // 停止扫描
  if (pBLEScan != nullptr && pBLEScan->isScanning())
  {
    pBLEScan->stop();
    console.log(sqINFO, "扫描已停止.");
    scanActive = false; // 标志位复位
  }

  // 释放 NimBLE 资源
  if (NimBLEDevice::getInitialized())
  {
    NimBLEDevice::deinit();
    console.log(sqINFO, "NimBLE 设备已释放资源.");
  }

  // 清理已发现的设备列表
  discoveredDevices.clear();
  console.log(sqINFO, "已清理发现的设备列表.");
}

// FreeRTOS 任务包装
void SQNimBLEService::bleTaskWrapper(void *pvParameters)
{
  SQNimBLEService *instance = static_cast<SQNimBLEService *>(pvParameters);
  instance->task(pvParameters); // 调用成员函数 task()
}

void SQNimBLEService::task(void *pvParameters)
{
  while (1)
  {
    processQueue(); // 处理队列中的消息

    // 只有在扫描时才进行扫描状态检查
    // If an error occurs that stops the scan, it will be restarted here.
    if (scanActive && !pBLEScan->isScanning())
    {
      // Start scan with: duration = 0 seconds(forever), no scan end callback, not a continuation of a previous scan.
      pBLEScan->start(0, nullptr, false); // 无限时长扫描
    }

    // 调整延迟时间，延长低负载时的休眠时间
    vTaskDelay(deviceConnected ? 100 / portTICK_PERIOD_MS : 500 / portTICK_PERIOD_MS); // 连接时更频繁地处理队列
  }
}

SQNimBLEService nimbleService; // 创建 NimBLE 服务的实例
NimBLEScan *pBLEScan = nullptr;

#endif // USE_NIMBLE