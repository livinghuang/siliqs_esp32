#pragma once
#include "bsp.h"
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#include "SoftwareSerial.h"

#ifdef USE_PM25
#ifdef USE_G5T_UART

#define MAX_PM2P5 1000
#define MIN_PM2P5 0
#define MAX_PM10 1000
#define MIN_PM10 0
#define G5T_BAUD_RATE 9600

// 定义 dsco2 数据结构体
struct g5t_data_v2
{
  uint16_t address;
  uint16_t length;
  uint16_t fpm1p0;
  uint16_t fpm2p5;
  uint16_t fpm10;
  uint16_t pm1p0;
  uint16_t pm2p5;
  uint16_t pm10;
  uint16_t um03;
  uint16_t um05;
  uint16_t um10;
  uint16_t um25;
  int16_t temperature;
  uint16_t humidity;
  uint8_t version;
  uint8_t errorCode;
  uint16_t checksum;
};

// 定义联合体，用于二进制数据的解析
union g5t_data_union
{
  uint16_t rawBytes[sizeof(g5t_data_v2) / 2];
  struct g5t_data_v2 data;
};

// Co2Measurement 类声明，继承自 Sensor
class PM25Measurement : public Sensor
{
private:
  SoftwareSerial softSerial;        // 软件串口实例
  union g5t_data_union g5t_data_v2; // 用于存储接收到的数据

public:
  int16_t pm2p5 = -1;
  int16_t pm10 = -1;
  bool busy = false;

  // 构造函数，传入 RX 和 TX 引脚
  PM25Measurement(uint8_t rxPin = pG5T_RX, uint8_t txPin = pG5T_TX);

  // 初始化方法，重写父类的 begin 方法
  void begin() override;
  void end();

  // 获取 CO2 测量值，重写父类的 getMeasurement 方法
  void getMeasurement() override;
  // 析构函数，用于销毁对象时的清理
  ~PM25Measurement();
};

#endif
#endif