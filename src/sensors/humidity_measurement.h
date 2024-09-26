#include "bsp.h"
#ifdef USE_HUMIDITY
#ifndef HUMIDITY_MEASUREMENT_H
#define HUMIDITY_MEASUREMENT_H
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#include "sensors/hdc1080/HDC1080.h"
#include <Wire.h>
#include "pins_defined.h"
#define HDC1080_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址

class HumidityMeasurement : public Sensor
{
private:
  int i2cAddress;
  int sdaPin;
  int sclPin;
  HDC1080 hdc1080;

public:
  float humidity; // 当前湿度值

  HumidityMeasurement(int i2cAddress = HDC1080_I2C_ADDRESS, int sdaPin = pSDA, int sclPin = pSCL, int powerPin = pVext);

  void begin() override;          // 初始化方法
  void getMeasurement() override; // 获取测量值
};

#endif
#endif