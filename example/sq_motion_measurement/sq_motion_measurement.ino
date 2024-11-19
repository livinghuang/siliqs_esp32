#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "sensors/motion_measurement.h"
/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
MotionMeasurement motionSensor(MPU6050_I2C_ADDRESS, pMPU6050_I2C_SDA, pMPU6050_I2C_SCL, -1);
void setup()
{
  Serial.begin(115200);
  siliqs_heltec_esp32_setup(SQ_INFO);
  Serial.println("Initializing motion sensor...");
  // motionSensor.begin();
  // Start the motion sensor task
  motionSensor.start(500,&i2cMutex);
}

void loop()
{
  // motionSensor.getMeasurement();
  Serial.println("ax: " + String(motionSensor.ax) + ", ay: " + String(motionSensor.ay) + ", az: " + String(motionSensor.az));
  Serial.println("gx: " + String(motionSensor.gx) + ", gy: " + String(motionSensor.gy) + ", gz: " + String(motionSensor.gz));
  delay(500); // Adjust the delay based on your application's needs
}
