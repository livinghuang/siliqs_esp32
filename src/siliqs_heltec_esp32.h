#pragma once

#include <Arduino.h>
#include "pins_defined.h"
#include "system/serial_console/serial_console.h"
#include "system/file_system/file_system.h"

#ifdef USE_NIMBLE
#include "system/nimble_service/nimble_service.h"
#endif

#ifdef USE_WIFI_CLIENT
#include "system/wifi_client/wifi_client.h"
#endif

#ifdef USE_WEB_OTA_SERVER
#include "system/web_ota_server/web_ota_server.h"
#endif

#ifdef USE_AT_COMMAND_SERVICE
#include "system/at_command_service/at_command_service.h"
#endif

#ifdef USE_LORAWAN
#include "system/lorawan_service/lorawan_service.h"
#endif

#ifdef USE_LORA
#include "system/lora_service/lora_service.h"
#endif

#ifdef USE_GPS
#include "system/gps_service/gps_service.h"
#endif

#ifdef USE_RGB_LED
#include "system/rgb_led_service/rgb_led_service.h"
#endif

extern SemaphoreHandle_t i2cMutex;
void siliqs_heltec_esp32_setup(int print_level = SQ_NONE);
uint64_t get_chip_id();
esp_sleep_wakeup_cause_t print_wakeup_reason(void);