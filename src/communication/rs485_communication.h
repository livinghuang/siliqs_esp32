#include "bsp.h"
#ifdef USE_RS485
#ifndef RS485_COMMUNICATION_H
#define RS485_COMMUNICATION_H
#include "siliqs_heltec_esp32.h"
#include "communication.h"
#include <HardwareSerial.h> // 用于 RS485 通信的硬件串口
#include "pins_defined.h"   // 包含引脚定义

// RS485 引脚配置和默认波特率
#define DEFAULT_RS485_BAUD_RATE 9600
#define MODBUS_MAX_DATA_LENGTH 255
typedef struct
{
  uint8_t address;
  uint8_t function;
  uint8_t data[MODBUS_MAX_DATA_LENGTH]; // Adjust the size as needed
  uint16_t crc;                         // CRC value
  uint8_t length;                       // Length of data
} modbus_t;

class RS485Communication : public Communication
{
public:
  // 构造函数，允许指定硬件串口，默认使用 Serial1
  RS485Communication(HardwareSerial &serial = Serial1, int baudRate = DEFAULT_RS485_BAUD_RATE, int RO = pRS485_RO, int DI = pRS485_DI, int directionPin = pRS485_DE, int powerPin = -1);

  // 重写基类方法
  void begin() override;                                                  // 初始化 RS485 通信
  void send(const char *data) override;                                   // 发送 RS485 数据
  void receive(char *buffer, int maxLength, int timeout = 1000) override; // 接收 RS485 数据
  void send_modbus(modbus_t *mb);
  void receive_modbus(modbus_t *mb, int timeout = 1000);
  // 覆写 powerOn 和 powerOff 方法
  void powerOn() override;
  void powerOff() override;

  // 析构函数，释放资源
  ~RS485Communication();

private:
  HardwareSerial &_serial; // 硬件串口的引用
  int _RO;                 // 接收数据引脚
  int _DI;                 // 发送数据引脚
  int _directionPin;       // 控制发送/接收模式的引脚
  int _baudRate;           // 串口波特率

  void enableTransmit(); // 切换到发送模式
  void enableReceive();  // 切换到接收模式
  void crc(modbus_t *mb);
  void print_modbus_frame(modbus_t *mb);
  void update_crc(uint16_t *crc);
};

#endif // RS485_COMMUNICATION_H
#endif // USE_RS485