#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "siliqs_heltec_esp32.h"
class Communication
{
protected:
  int powerPin; // 电源引脚

public:
  // 构造函数，允许初始化电源引脚
  Communication(int pin = -1) : powerPin(pin) {}

  virtual void begin() = 0;                           // 声明虚拟 begin 方法
  virtual void send(const char *data) = 0;            // 声明虚拟 send 方法
  virtual void receive(char *buffer, int length) = 0; // 声明虚拟 receive 方法

  // 电源控制方法
  virtual void powerOn()
  {
    if (powerPin != -1)
    {
      pinMode(powerPin, OUTPUT);
      digitalWrite(powerPin, HIGH); // 打开电源
    }
  }

  virtual void powerOff()
  {
    if (powerPin != -1)
    {
      digitalWrite(powerPin, LOW); // 关闭电源
    }
  }

  virtual ~Communication() // 虚拟析构函数，确保子类正确销毁
  {
    powerOff(); // 在析构时关闭电源
  }
};

#endif // COMMUNICATION_H