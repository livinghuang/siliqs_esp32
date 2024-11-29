#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "siliqs_heltec_esp32.h"
class Communication
{
protected:
  int _powerPin; // 电源引脚
  bool _powerPinHighActive = true;

public:
  // 构造函数，允许初始化电源引脚
  Communication(int pin = -1, bool highActive = true) : _powerPin(pin), _powerPinHighActive(highActive) {}

  virtual void begin() = 0;                                                    // 声明虚拟 begin 方法
  virtual void send(const char *data, int length) = 0;                         // 声明虚拟 send 方法
  virtual size_t receive(char *buffer, size_t length, int timeout = 1000) = 0; // 声明虚拟 receive 方法

  // 电源控制方法
  void powerOn()
  {
    if (_powerPin != -1)
    {
      pinMode(_powerPin, OUTPUT);
      if (_powerPinHighActive)
      {
        digitalWrite(_powerPin, HIGH); // 打开电源
      }
      else
      {
        digitalWrite(_powerPin, LOW); // 打开电源
      }
      while (1)
      {
        Serial.println("waiting for power");
        delay(1000);
      }
    }
  }
  void powerOff()
  {
    if (_powerPin != -1)
    {
      if (_powerPinHighActive)
      {
        digitalWrite(_powerPin, LOW); // 關閉电源
      }
      else
      {
        digitalWrite(_powerPin, HIGH); // 關閉电源
      }
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