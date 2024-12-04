#pragma once
#include "bsp.h"
#ifdef USE_HUMIDITY
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#include "sensors/hdc1080/HDC1080.h"
#include <Wire.h>
#define HUMIDITY_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址

class HumidityMeasurement : public Sensor
{
private:
  HDC1080 hdc1080;

public:
  float humidity = -1; // 当前湿度值

  void begin() override;          // 初始化方法
  void getMeasurement() override; // 获取测量值
};
#endif