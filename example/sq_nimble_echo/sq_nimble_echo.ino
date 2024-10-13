#include "bsp.h"
#include "siliqs_heltec_esp32.h"

void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO);
  // Initialize the NimBLE service
  nimble_setup();
}

void loop()
{
  nimble_loop();
}