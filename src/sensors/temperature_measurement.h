#include "bsp.h"
#ifdef USE_TEMPERATURE
#ifndef TEMPERATURE_MEASUREMENT_H
#define TEMPERATURE_MEASUREMENT_H
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#include <Wire.h>
#include "sensors/hdc1080/HDC1080.h"
#include "pins_defined.h"
#define HDC1080_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址

class TemperatureMeasurement : public Sensor
{
private:
  int i2cAddress;  // I2C 设备地址
  int sdaPin;      // SDA 引脚
  int sclPin;      // SCL 引脚
  HDC1080 hdc1080; // HDC1080 对象

public:
  float temperature; // 当前温度值

  // 构造函数，传入 I2C 地址、SDA 和 SCL 引脚及可选电源引脚
  TemperatureMeasurement(int i2cAddress = HDC1080_I2C_ADDRESS, int sdaPin = pSDA, int sclPin = pSCL, int powerPin = pVext);
  // 初始化方法
  void begin() override;

  // 实现获取测量值的方法
  void getMeasurement() override;
};

#endif
#endif