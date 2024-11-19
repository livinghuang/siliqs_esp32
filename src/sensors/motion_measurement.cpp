#pragma once
#include "bsp.h"

#ifdef USE_MOTION
#include "siliqs_heltec_esp32.h"
#include "motion_measurement.h"
#include <Wire.h>
#include "sensors/mpu6050/Mpu6050.h"
#include "sensors/mpu6050/I2Cdev.h"

MotionMeasurement::MotionMeasurement(uint8_t address, int sda, int scl, int power)
    : addr(address), sdaPin(sda), sclPin(scl), powerPin(power)
{
}
MotionMeasurement::~MotionMeasurement()
{ // Use scope resolution operator here
  if (MPU)
  {
    delete MPU; // Clean up dynamically allocated memory
    MPU = nullptr;
  }
}

void MotionMeasurement::begin()
{
  Wire.begin(sdaPin, sclPin, 100000);
  MPU = new MPU6050_Base(addr); // Assuming dynamic allocation
  MPU->initialize();

  if (!MPU->testConnection())
  {
    console.log(sqINFO, "MPU6050 connection failed!");
  }
  else
  {
    console.log(sqINFO, "MPU6050 connected successfully.");
  }

  if (powerPin != -1)
  {
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH); // Power on the sensor
  }
}

void MotionMeasurement::getMeasurement()
{
  MPU->getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  console.log(sqINFO, "Accel: [%d, %d, %d], Gyro: [%d, %d, %d]\n", ax, ay, az, gx, gy, gz);
}

#endif
