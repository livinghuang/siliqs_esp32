#pragma once
#include "bsp.h"
#ifdef USE_TEMPERATURE
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#ifdef USE_HDC1080_I2C
#include <Wire.h>
#include "sensors/hdc1080/HDC1080.h"

class TemperatureMeasurement : public Sensor
{
private:
  HDC1080 hdc1080; // HDC1080 对象

public:
  float temperature = -273.15; // 当前温度值

  // 初始化方法
  void begin() override;

  // 实现获取测量值的方法
  void getMeasurement() override;
};
#endif
#endif