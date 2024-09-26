#include "bsp.h"
#ifdef USE_HUMIDITY
#include "humidity_measurement.h"

HumidityMeasurement::HumidityMeasurement(int i2cAddress, int sdaPin, int sclPin, int powerPin)
    : Sensor(powerPin), i2cAddress(i2cAddress), sdaPin(sdaPin), sclPin(sclPin), humidity(0) {}

// 初始化传感器
void HumidityMeasurement::begin()
{
  Sensor::begin();
  hdc1080.begin(sdaPin, sclPin); // 使用指定的 SDA 和 SCL 引脚初始化 HDC1080
}

// 获取湿度值
void HumidityMeasurement::getMeasurement()
{
  if (powerPin != -1)
  {
    powerOn();
  }

  // heat up
  hdc1080.heatUp(10);
  humidity = hdc1080.readHumidity(); // 调用 HDC1080 库获取湿度值

  if (powerPin != -1)
  {
    powerOff();
  }
}
#endif