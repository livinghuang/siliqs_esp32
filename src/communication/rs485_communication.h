#include "bsp.h"
#ifdef USE_RS485
#ifndef RS485_COMMUNICATION_H
#define RS485_COMMUNICATION_H
#include "siliqs_esp32.h"
#include "communication.h"
#include <HardwareSerial.h> // 用于 RS485 通信的硬件串口
#include "pins_defined.h"   // 包含引脚定义

#define DEFAULT_RS485_BAUD_RATE 9600
#define RS485_MAX_DATA_LENGTH 255

class RS485Communication
{
public:
  // 构造函数，允许指定硬件串口，默认使用 Serial1
  RS485Communication(HardwareSerial &serial = Serial1, int baudRate = DEFAULT_RS485_BAUD_RATE, int RO = pRS485_RO, int DI = pRS485_DI, int directionPin = pRS485_DE, int powerPin = -1, bool highActive = true);

  // 重写基类方法
  void begin();                                                                                                // 初始化 RS485 通信
  void send(const char *data, int length);                                                                     // 发送 RS485 数据
  size_t receive(char *buffer, size_t length = RS485_MAX_DATA_LENGTH, int timeout = -1);                       // 接收 RS485 数据
  size_t readFromChar(char *buffer, char start_char, size_t length = RS485_MAX_DATA_LENGTH, int timeout = -1); // 接收 RS485 数据
  void print(const String &data);
  void println(const String &data);
  String readStringUntil(char end);

  void setReceiveTimeout(int timeout);
  void powerOn();
  void powerOff();

  // 析构函数，释放资源
  ~RS485Communication();

protected:
  HardwareSerial &_serial; // 硬件串口的引用
  int _RO;                 // 接收数据引脚
  int _DI;                 // 发送数据引脚
  int _directionPin;       // 控制发送/接收模式的引脚
  int _baudRate;           // 串口波特率
  int _powerPin;
  bool _powerPinHighActive;

  void enableTransmit(); // 切换到发送模式
  void enableReceive();  // 切换到接收模式
};
void print_bytes(const uint8_t *data, int length);
#endif // RS485_COMMUNICATION_H
#endif // USE_RS485