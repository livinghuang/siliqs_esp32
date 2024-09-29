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

  virtual void begin() = 0;                                                    // 声明虚拟 begin 方法
  virtual void send(const char *data, int length) = 0;                         // 声明虚拟 send 方法
  virtual size_t receive(char *buffer, size_t length, int timeout = 1000) = 0; // 声明虚拟 receive 方法

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

protected:
  void print_bytes(const uint8_t *data, int length)
  {
    for (int i = 0; i < length; i++)
    {
      // Print each byte in hexadecimal format with leading zeros
      if (data[i] < 0x10)
      {
        Serial.print("0");
      }
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println(); // Print a newline character at the end
  }
};

#endif // COMMUNICATION_H