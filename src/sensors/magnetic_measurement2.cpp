#include "bsp.h"
#ifdef USE_MAGNETOMETER2
#include "magnetic_measurement2.h"
#include "Arduino.h"
Magnetometer2 mag(QMC5883_ADDRESS, pMQC5883_I2C_SDA, pMQC5883_I2C_SCL);
void test_magnetic()
{
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  delay(1000);
  Wire.begin(pMQC5883_I2C_SDA, pMQC5883_I2C_SCL, 400000);
  mag.begin(Wire);
  while (1)
  {
    mag.getMeasurement();
    if (mag.isAlarm())
    {
      if (mag.alarmOverflow)
        Serial.println("Alarm overflow");
      if (mag.alarmCommError)
        Serial.println("Alarm comm error");
    }
    else
    {
      Serial.print("X: ");
      Serial.print(mag.mGaussX);
      Serial.println(" mGauss");
      Serial.print(" Y: ");
      Serial.print(mag.mGaussY);
      Serial.println(" mGauss");
      Serial.print(" Z: ");
      Serial.print(mag.mGaussZ);
      Serial.println(" mGauss");
    }
    delay(200);
  }
}
#endif