#include "bsp.h"
#ifdef USE_DSP310_I2C
#include "air_pressure_measurement.h"

// 初始化传感器
void AirPressureMeasurement::begin()
{
  Wire.setPins(pDSP310_I2C_SDA, pDSP310_I2C_SCL);
  Dps310PressureSensor.begin(Wire, DSP310_I2C_ADDRESS);
}

// 获取气压测量值
void AirPressureMeasurement::getMeasurement()
{
  uint8_t oversampling = 7;
  int16_t ret = Dps310PressureSensor.measurePressureOnce(pressure, oversampling);
  float altitude = 44330.0 * (1.0 - pow(pressure / 101325, 0.1903));
  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }

  ret = Dps310PressureSensor.measureTempOnce(temperature, oversampling);

  if (ret != 0)
  {
    // Something went wrong.
    // Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
}
#endif