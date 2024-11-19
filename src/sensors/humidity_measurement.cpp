#include "bsp.h"
#ifdef USE_HUMIDITY
#ifdef USE_HDC1080_I2C
#include "humidity_measurement.h"

// 初始化传感器
void HumidityMeasurement::begin()
{
  hdc1080.begin(pHDC1080_I2C_SDA, pHDC1080_I2C_SCL); // 使用指定的 SDA 和 SCL 引脚初始化 HDC1080
}

// 获取湿度值
void HumidityMeasurement::getMeasurement()
{
  // heat up
  hdc1080.heatUp(HDC1080_HEATER_ON_SEC);
  humidity = hdc1080.readHumidity(); // 调用 HDC1080 库获取湿度值
}
#endif
#endif