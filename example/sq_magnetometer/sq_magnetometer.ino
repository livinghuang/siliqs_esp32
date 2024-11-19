#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "sensors/magnetic_measurement.h"
/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */

Magnetometer mag(MQC5883_I2C_ADDRESS, pMQC5883_I2C_SDA, pMQC5883_I2C_SCL); // Address, SDA pin, SCL pin

void setup()
{
  Serial.begin(115200);
  siliqs_heltec_esp32_setup(SQ_INFO);
  mag.begin();
}

void loop()
{
  mag.getMeasurement();
  delay(1000);
}
