#include "bsp.h"
#ifdef USE_TEMPERATURE
#include "temperature_measurement.h"
TemperatureMeasurement::TemperatureMeasurement(int i2cAddress, int sdaPin, int sclPin, int powerPin)
    : Sensor(powerPin), i2cAddress(i2cAddress), sdaPin(sdaPin), sclPin(sclPin), temperature(0) {}

// 初始化传感器
void TemperatureMeasurement::begin()
{
  Sensor::begin();
  hdc1080.begin(sdaPin, sclPin); // 明确调用带有 SDA 和 SCL 引脚的 begin()
}

// 获取温度值
void TemperatureMeasurement::getMeasurement()
{
  if (powerPin != -1)
  {
    powerOn();
  }

  if (!hdc1080.begin())
  {
    Serial.println("Could not find a valid HDC1080 sensor, check wiring!");
  }
  else
  {
    temperature = hdc1080.readTemperature();
  }

  if (powerPin != -1)
  {
    powerOff();
  }
}

#endif