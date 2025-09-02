#include "bsp.h"
#include "siliqs_esp32.h"

I2CCommunication i2c;

void setup()
{
  siliqs_esp32_setup();
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, LOW); // Power on
  delay(200);
  Serial.println("Boot");
}

void loop()
{
  // 會印出總線上回應的 7-bit 位址
  i2c.scan_bus();
  delay(3000);
}