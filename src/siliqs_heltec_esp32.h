#ifndef _SILIQS_HELTEC_ESP32_H_
#define _SILIQS_HELTEC_ESP32_H_

#include <Arduino.h>
#include "heltec.h"
#include "pins_defined.h"
#include "system/serial_console/serial_console.h"

void siliqs_heltec_esp32_setup(int print_level = SQ_NONE);
uint64_t get_chip_id();
esp_sleep_wakeup_cause_t print_wakeup_reason(void);
// void print_bytes(const uint8_t *data, int length);
// void print_bytes_reverse(uint8_t *data, int length);
#endif