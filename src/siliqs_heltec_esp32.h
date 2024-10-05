#ifndef _SILIQS_HELTEC_ESP32_H_
#define _SILIQS_HELTEC_ESP32_H_

#include <Arduino.h>
#include "heltec.h"
#include "pins_defined.h"
#include "system/serial_console/serial_console.h"
#ifdef USE_BLE
#include "system/ble_service/ble_service.h"
#endif
void siliqs_heltec_esp32_setup(int print_level = SQ_NONE);
uint64_t get_chip_id();
esp_sleep_wakeup_cause_t print_wakeup_reason(void);
#endif