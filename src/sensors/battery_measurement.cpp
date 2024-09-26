#include "bsp.h"
#ifdef USE_BATTERY
#include "battery_measurement.h"

BatteryMeasurement::BatteryMeasurement(int analogPin, int powerPin)
    : Sensor(powerPin), analogPin(analogPin), batteryVoltage(0), batteryPercentage(0) {}

// 初始化传感器
void BatteryMeasurement::begin()
{
  Sensor::begin(); // 调用基类的初始化方法
}

// 获取测量值
void BatteryMeasurement::getMeasurement()
{
  if (powerPin != -1)
  {
    powerOn(); // 确保电源打开
  }

  uint32_t sum = 0;
  for (size_t i = 0; i < 16; i++)
  {
    uint16_t analogValue = analogRead(analogPin); // 读取模拟值
    sum += analogValue;
  }

  float avg = (float)(sum >> 4); // 计算16次采样的平均值

  // 使用宏定义替换硬编码的值
  batteryVoltage = ((avg - TESTED_MIN) * (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) / (TESTED_MAX - TESTED_MIN) + BATTERY_MIN_VOLTAGE);

  // 计算电池百分比
  batteryPercentage = (batteryVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100;
  if (batteryPercentage > 100)
    batteryPercentage = 100;
  if (batteryPercentage < 0)
    batteryPercentage = 0;

  if (powerPin != -1)
  {
    powerOff(); // 获取测量值后关闭电源
  }
}

#endif