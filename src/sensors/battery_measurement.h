#include "bsp.h"
#ifdef USE_BATTERY
#ifndef BATTERY_MEASUREMENT_H
#define BATTERY_MEASUREMENT_H

#include "sensor_measurement.h"
#include "pins_defined.h"
#define TESTED_MIN 1970
#define TESTED_MAX 2880
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0

class BatteryMeasurement : public Sensor
{
private:
  int analogPin; // 模拟输入引脚

public:
  float batteryVoltage;  // 电池电压
  int batteryPercentage; // 电池电量

  // 构造函数，传入模拟引脚和可选电源引脚
  BatteryMeasurement(int analogPin = pADC_BAT, int powerPin = -1);

  // 初始化方法
  void begin() override;

  // 实现获取测量值的方法
  void getMeasurement() override;
};
#endif
#endif