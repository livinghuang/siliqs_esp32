#pragma once
#include "bsp.h"
#ifdef USE_BATTERY
#include "sensor_measurement.h"
#ifdef USE_BAT_ADC

class BatteryMeasurement : public Sensor
{
public:
  float batteryVoltage;  // 电池电压
  int batteryPercentage; // 电池电量

  // 初始化方法
  void begin() override;

  // 实现获取测量值的方法
  void getMeasurement() override;
};
#endif
#endif