#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "sensors/battery_measurement.h"

BatteryMeasurement battery;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 * 然后，调用 battery.begin() 函数来初始化电池测量系统。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  // 初始化电池测量系统
  battery.begin();
}

void loop()
{
  delay(1000);
  // 获取电池测量值
  battery.getMeasurement();
  Serial.print("Battery Voltage: ");
  Serial.println(battery.batteryVoltage);

  Serial.print("Battery Percentage: ");
  Serial.println(battery.batteryPercentage);
  delay(1000);
}
