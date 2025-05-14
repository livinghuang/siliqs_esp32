#include "bsp.h"
#include "siliqs_esp32.h"
// you have to use deep sleep otherwise the RGB LED won't work

RGBLedService rgb_led;
void setup()
{
  // Initialize system and LED peripherals
  siliqs_esp32_setup(SQ_INFO);

  Serial.println("RMT initialized with 100ns tick");
  rgb_led.begin(pRGB_LED, 1); // Initialize the service
}

void loop()
{
  int color_red[] = {0x00, 0xFF, 0x00};
  int color_blue[] = {0x00, 0x00, 0xFF};
  int color_green[] = {0xFF, 0x00, 0x00};
  int color_white[] = {0xFF, 0xFF, 0xFF};
  int color_off[] = {0x00, 0x00, 0x00};
  rgb_led.fillLedData(color_red, 0);
  rgb_led.light();
  delay(1000);

  rgb_led.fillLedData(color_blue, 0);
  rgb_led.light();
  delay(1000);

  rgb_led.fillLedData(color_green, 0);
  rgb_led.light();
  delay(1000);

  rgb_led.fillLedData(color_white, 0);
  rgb_led.light();
  delay(1000);

  rgb_led.fillLedData(color_off, 0);
  rgb_led.light();
  delay(1000);

  rgb_led.end();
  delay(1000);
  gotoSleep(10); // you have to use deep sleep otherwise the RGB LED won't work
}
