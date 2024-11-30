#include "bsp.h"
#include "siliqs_esp32.h"
#include "sensors/temperature_measurement.h"

TemperatureMeasurement tempSensor;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup();
  // 初始化温度测量系统
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, LOW); // Power on
  tempSensor.begin();
}

void loop()
{
  delay(1000); // wait for sensor to be ready
  // 获取温度测量值
  tempSensor.getMeasurement();
  Serial.print("Temperature: ");
  Serial.print(tempSensor.temperature);
  Serial.println(" C");
}
