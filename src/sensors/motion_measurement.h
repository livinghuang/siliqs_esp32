#pragma once
#include "bsp.h"

#ifdef USE_MOTION
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#include <Wire.h>
#include "sensors/mpu6050/Mpu6050.h"
#include "sensors/mpu6050/I2Cdev.h"

class MotionMeasurement : public Sensor
{
public:
  MotionMeasurement(uint8_t address = MPU6050_I2C_ADDRESS, int sda = pMPU6050_I2C_SDA, int scl = pMPU6050_I2C_SCL, int powerPin = -1);
  ~MotionMeasurement();
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  // 初始化方法
  void begin() override;

  // 实现获取测量值的方法
  void getMeasurement() override;

private:
  uint8_t addr;
  int sdaPin;
  int sclPin;
  int powerPin;
  MPU6050_Base *MPU;
};

#endif
