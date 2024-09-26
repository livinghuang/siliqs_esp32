#include "bsp.h"
#ifdef USE_AIR_PRESSURE
#include "air_pressure_measurement.h"

AirPressureMeasurement::AirPressureMeasurement(int i2cAddress, int sdaPin, int sclPin, int powerPin)
    : Sensor(powerPin), i2cAddress(i2cAddress), sdaPin(sdaPin), sclPin(sclPin), pressure(0) {}

// 初始化传感器
void AirPressureMeasurement::begin()
{
  Sensor::begin();
  Wire.setPins(pSDA, pSCL);
  Dps310PressureSensor.begin(Wire, 0x77);
}

// 获取气压测量值
void AirPressureMeasurement::getMeasurement()
{
  if (powerPin != -1)
  {
    powerOn(); // 确保传感器通电
  }

  uint8_t oversampling = 7;
  int16_t ret = Dps310PressureSensor.measurePressureOnce(pressure, oversampling);
  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  // else
  // {
  //   Serial.print("Pressure: ");
  //   Serial.print(pressure);
  //   Serial.println(" Pascal");
  // }

  ret = Dps310PressureSensor.measureTempOnce(temperature, oversampling);

  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  // else
  // {
  //   Serial.print("Temperature: ");
  //   Serial.print(temperature);
  //   Serial.println(" degrees of Celsius");
  // }

  if (powerPin != -1)
  {
    powerOff(); // 读取后关闭电源以节约电力
  }
}

#endif