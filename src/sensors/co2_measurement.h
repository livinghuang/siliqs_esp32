#pragma once
#include "bsp.h"
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#include "SoftwareSerial.h"

#ifdef USE_CO2
#ifdef USE_DSCO2_UART

#define MAX_CO2 4000
#define MIN_CO2 0
#define DSCO2_BAUD_RATE 9600

// 定义 dsco2 数据结构体
struct dsco2_data_v2
{
  uint16_t address;
  uint16_t length;
  int16_t co2;
  uint16_t param1;
  uint16_t param2;
  uint16_t checksum;
};

// 定义联合体，用于二进制数据的解析
union dsco2_data_union
{
  uint16_t rawBytes[sizeof(dsco2_data_v2) / sizeof(uint16_t)];
  struct dsco2_data_v2 data;
};

// Co2Measurement 类声明，继承自 Sensor
class Co2Measurement : public Sensor
{
private:
  SoftwareSerial softSerial;            // 软件串口实例
  union dsco2_data_union dsco2_data_v2; // 用于存储接收到的数据

public:
  int16_t co2 = -1; // CO2 浓度
  bool busy = false;

  // 构造函数，传入 RX 和 TX 引脚
  Co2Measurement(uint8_t rxPin = pDSCO2_RX, uint8_t txPin = pDSCO2_TX);

  // 初始化方法，重写父类的 begin 方法
  void begin() override;
  void end();

  // 获取 CO2 测量值，重写父类的 getMeasurement 方法
  void getMeasurement() override;
  // 析构函数，用于销毁对象时的清理
  ~Co2Measurement();
};

#endif
#endif