// #include "bsp.h"

// #ifdef USE_BLE // 仅在启用 USE_BLE 时编译
// #include "ble_service.h"
// #include <Arduino.h>

// void UartCallbacks ::onWrite(BLECharacteristic *pCharacteristic)
// { // 获取接收的数据并将其存储到 SQ_BLEService 的成员变量中
//   SQ_BLEService.setReceivedData(pCharacteristic->getValue().c_str());
//   console.log(sqINFO, "Received UART Value: " + pCharacteristic->getValue());
// }

// void SQ_BLEServiceClass::sendData(const char *data)
// {
//   if (deviceConnected)
//   {
//     // 设置特性值并发送通知
//     pTxCharacteristic->setValue((uint8_t *)data, strlen(data));
//     pTxCharacteristic->notify(); // 向所有连接的客户端发送通知
//     vTaskDelay(10);              // 避免堆栈拥塞
//     console.log(sqINFO, "Data sent to client.");
//   }
// }

// void SQ_BLEServiceClass::sendData(String data)
// {
//   sendData(data.c_str());
// }

// void SQ_BLEServiceClass::setReceivedData(const String &data)
// {
//   receivedData = data;
// }
// // 获取接收到的数据
// String SQ_BLEServiceClass::getReceivedData()
// {
//   return receivedData;
// }

// #endif // USE_BLE 宏结束