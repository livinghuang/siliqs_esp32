#include "bsp.h"
#include "siliqs_heltec_esp32.h"

RGBLedService led(pRGB_LED, 1);
void setup()
{
  // Initialize system and LED peripherals
  siliqs_heltec_esp32_setup(SQ_INFO);

  led.begin(); // Initialize the service

  Serial.println("RMT initialized with 100ns tick");
}

void loop()
{
  int color_green[] = {0x00, 0xFF, 0x00};
  int color_blue[] = {0x00, 0x00, 0xFF};
  int color_red[] = {0xFF, 0x00, 0x00};
  int color_white[] = {0xFF, 0xFF, 0xFF};
  int color_off[] = {0x00, 0x00, 0x00};

  led.set_led(color_green);
  delay(1000);
  led.set_led(color_blue);
  delay(1000);
  led.set_led(color_red);
  delay(1000);
  led.set_led(color_white);
  delay(1000);
  led.set_led(color_off);
  delay(1000);
}
