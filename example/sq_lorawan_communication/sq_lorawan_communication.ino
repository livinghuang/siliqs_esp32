#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/lorawan_communication.h"

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_heltec_esp32_setup();
  lorawanSetup();
}

void loop()
{
  lorawanLoop();
}
