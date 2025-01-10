#include "bsp.h"
#include "siliqs_esp32.h"
String toFindDeviceName = "YT9";
String toFindDeviceAddress = "d5:12:d8:96:00:31";
void setup()
{
  siliqs_esp32_setup(SQ_INFO);

  Serial.println("初始化 NimBLE 服务...");

  // 初始化 NimBLE 服务
  nimbleService.init();

  nimbleService.startScanDevices(0, toFindDeviceName, toFindDeviceAddress); // Start scan with: duration = 0 seconds(forever), no scan end callback, not a continuation of a previous scan.
  Serial.println("NimBLE 服务初始化完成，开始扫描...");
}

void loop()
{
  static uint32_t time = millis();
  if (millis() - time > 5000)
  {
    time = millis();
    // nimbleService.printDiscoveredDevices();
    if (nimbleService.deviceFoundWhenScanning)
    {
      Serial.println("设备已找到，停止扫描...");
      nimbleService.stopScanDevices();
      Serial.println(nimbleService.foundDevice.toString().c_str());
      nimbleService.rescanDevices(0, toFindDeviceName, toFindDeviceAddress);
    }
  }
}