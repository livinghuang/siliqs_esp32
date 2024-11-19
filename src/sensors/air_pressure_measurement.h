#pragma once
#include "bsp.h"
#ifdef USE_AIR_PRESSURE
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#ifdef USE_DSP310_I2C
#include <Wire.h>
#include "sensors/dsp310/Dps310.h"
class AirPressureMeasurement : public Sensor
{
private:
  Dps310 Dps310PressureSensor = Dps310();

public:
  float pressure = 0; // 当前气压值
  float temperature = -273.15;
  float altitude;

  void begin() override;          // 初始化方法
  void getMeasurement() override; // 获取测量值
};
#endif
#endif
