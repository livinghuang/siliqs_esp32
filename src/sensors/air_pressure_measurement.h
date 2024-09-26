#include "bsp.h"
#ifdef USE_AIR_PRESSURE
#ifndef AIR_PRESSURE_MEASUREMENT_H
#define AIR_PRESSURE_MEASUREMENT_H
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#include <Wire.h>
#include "sensors/dsp310/Dps310.h"
#include "pins_defined.h"
#define DSP310_I2C_ADDRESS 0x77 // 假设使用 HDC1080 传感器的 I2C 地址

class AirPressureMeasurement : public Sensor
{
private:
  int i2cAddress; // I2C 设备地址
  int sdaPin;     // SDA 引脚
  int sclPin;     // SCL 引脚
  Dps310 Dps310PressureSensor = Dps310();

public:
  float pressure; // 当前气压值
  float temperature;
  float altitude;

  // 构造函数，传入 I2C 地址、SDA 和 SCL 引脚及可选电源引脚
  AirPressureMeasurement(int i2cAddress = DSP310_I2C_ADDRESS, int sdaPin = pSDA, int sclPin = pSCL, int powerPin = pVext);

  void begin() override;          // 初始化方法
  void getMeasurement() override; // 获取测量值
};

#endif
#endif
