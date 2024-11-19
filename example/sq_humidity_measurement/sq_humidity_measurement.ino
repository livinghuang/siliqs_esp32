#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "sensors/humidity_measurement.h"

HumidityMeasurement humiditySensor;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  // 初始化湿度测量系统
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, LOW); // Power on
  humiditySensor.begin();
}

void loop()
{
  delay(1000); // wait for sensor to be ready
  // 获取湿度测量值
  humiditySensor.getMeasurement();
  Serial.print("Humidity: ");
  Serial.print(humiditySensor.humidity);
  Serial.println(" %");
}
