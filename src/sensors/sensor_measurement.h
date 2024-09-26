#ifndef SENSOR_H
#define SENSOR_H
#include "siliqs_heltec_esp32.h"
#include "Arduino.h"

// 基类：带电源控制的通用传感器类
class Sensor
{
protected:
  int powerPin; // 电源引脚

public:
  // 构造函数，带可选电源引脚
  Sensor(int powerPin = -1);

  // 初始化传感器的通用方法
  virtual void begin();

  // 获取测量值的统一接口，由子类实现
  virtual void getMeasurement() = 0;

  // 电源控制方法
  virtual void powerOn();
  virtual void powerOff();

  // 虚析构函数
  virtual ~Sensor() {}
};

#endif