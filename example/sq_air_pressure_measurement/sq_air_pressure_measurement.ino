#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "sensors/air_pressure_measurement.h"

AirPressureMeasurement airPressureSensor;

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  // 初始化气压
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, LOW); // Power on
  airPressureSensor.begin();
}

void loop()
{
  delay(1000); // wait for sensor to be ready
  // 获取气压测量值
  airPressureSensor.getMeasurement();
  Serial.print("Air Pressure: ");
  Serial.print(airPressureSensor.pressure);
  Serial.println(" hPa");
  Serial.print("DSP310 Temperature: ");
  Serial.print(airPressureSensor.temperature);
  Serial.println(" degrees of Celsius");
}
