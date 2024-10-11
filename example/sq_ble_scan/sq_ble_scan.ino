#include "bsp.h"
#include "siliqs_heltec_esp32.h"

void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO);
#ifdef USE_BLE
  Serial.println("Start BLE service");
  // 初始化 BLE 服务
  SQ_BLEService.init(30000, "SQ-"); // your could name the prefix of your device, it should looks like "SQ-xxxx"
  // 创建 BLE 服务的 FreeRTOS 任务
  xTaskCreate(SQ_BLEServiceClass::bleTaskWrapper, "bleTask", 4096, NULL, 1, NULL);
  SQ_BLEService.startDiscovery(0); // scan 5 seconds and restart the scan, till call stopDiscovery() to stop. Set second over 0, will scan the specified seconds once.
#endif
}

void loop()
{
#ifdef USE_BLE
  if (SQ_BLEService.deviceConnected)
  {
    String data = SQ_BLEService.getReceivedData();
    if (data.length() > 0)
    {
      Serial.println("Received from client: " + data);
      // 处理接收到的数据...
      SQ_BLEService.setReceivedData(""); // 清空接收缓冲区
      SQ_BLEService.sendData(data);      // 发送数据 echo back
    }
  }

  if (SQ_BLEService.discoveredDevices.size() == 0)
  {
    Serial.println("No device found");
  }
  else
  {
    for (int i = 0; i < SQ_BLEService.discoveredDevices.size(); i++)
    {
      Serial.println(SQ_BLEService.discoveredDevices[i].toString());
    }
  }
  delay(10000);
#endif
}