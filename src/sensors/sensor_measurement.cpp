#include "sensor_measurement.h"

// 构造函数，初始化电源引脚
Sensor::Sensor(int powerPin) : powerPin(powerPin) {}

// 初始化方法，子类可以调用它
void Sensor::begin()
{
  if (powerPin != -1)
  {
    pinMode(powerPin, OUTPUT); // 设置电源引脚为输出模式
    powerOn();                 // 默认在初始化时开启电源
  }
}

// 开启电源的方法
void Sensor::powerOn()
{
  if (powerPin != -1)
  {
    digitalWrite(powerPin, LOW); // 设置引脚为高电平，开启电源
  }
}

// 关闭电源的方法
void Sensor::powerOff()
{
  if (powerPin != -1)
  {
    digitalWrite(powerPin, HIGH); // 设置引脚为低电平，关闭电源
  }
}