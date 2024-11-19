#include "bsp.h"
#ifdef USE_TEMPERATURE
#ifdef USE_HDC1080_I2C
#include "temperature_measurement.h"
// 初始化传感器
void TemperatureMeasurement::begin()
{
  hdc1080.begin(pHDC1080_I2C_SDA, pHDC1080_I2C_SCL); // 明确调用带有 SDA 和 SCL 引脚的 begin()
}

// 获取温度值
void TemperatureMeasurement::getMeasurement()
{
  if (!hdc1080.begin())
  {
    Serial.println("Could not find a valid HDC1080 sensor, check wiring!");
  }
  else
  {
    temperature = hdc1080.readTemperature();
  }
}
#endif
#endif