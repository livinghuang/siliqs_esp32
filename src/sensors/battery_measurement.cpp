#include "bsp.h"
#ifdef USE_BATTERY
#include "battery_measurement.h"
#ifdef USE_BAT_ADC
#define TESTED_MIN 1970
#define TESTED_MAX 2800
// 初始化传感器
void BatteryMeasurement::begin()
{
}

// 获取测量值
void BatteryMeasurement::getMeasurement()
{
  uint32_t sum = 0;
  for (size_t i = 0; i < 16; i++)
  {
    uint16_t analogValue = analogRead(BAT_ADC_PIN); // 读取模拟值
    sum += analogValue;
  }

  float avg = (float)(sum >> 4); // 计算16次采样的平均值
  console.log(sqINFO, "Battery ADC value: " + String(avg));
  // 使用宏定义替换硬编码的值
  batteryVoltage = ((avg - TESTED_MIN) * (BAT_MAX_VOLTAGE - BAT_MIN_VOLTAGE) / (TESTED_MAX - TESTED_MIN) + BAT_MIN_VOLTAGE) * BAT_VOLTAGE_MULTIPLIER;
  console.log(sqINFO, "Battery Voltage: " + String(batteryVoltage));
  // 计算电池百分比
  batteryPercentage = (batteryVoltage - BAT_MIN_VOLTAGE) / (BAT_MAX_VOLTAGE - BAT_MIN_VOLTAGE) * 100.0;
  if (batteryPercentage > 100)
    batteryPercentage = 100;
  if (batteryPercentage < 0)
    batteryPercentage = 0;
  console.log(sqINFO, "Battery Percentage: " + String(batteryPercentage));
}
#endif
#endif