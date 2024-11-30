#include "bsp.h"
#include "siliqs_esp32.h"
#include "sensors/ptc_measurement.h"

// Create an instance of PTCMeasurement
PTCMeasurement ptcSensor(pCS, pMOSI, pMISO, pSCK);

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup(SQ_INFO);
  // 初始化温度测量系统
  // Initialize the PTC sensor
  pinMode(pVext, OUTPUT);
}

void loop()
{
  // // Fetch and log temperature measurements
  digitalWrite(pVext, LOW);
  ptcSensor.begin();
  ptcSensor.getMeasurement();
  delay(1000); // Wait for 1 second between readings
  float temperature = ptcSensor.getTemperature();
  digitalWrite(pVext, HIGH);
  Serial.begin(115200);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
}
