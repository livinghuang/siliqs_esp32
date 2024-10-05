#include "siliqs_heltec_esp32.h"
void siliqs_heltec_esp32_setup(int print_level)
{
  Serial.begin(115200);
  console.begin(print_level);

#ifdef USE_BLE
  console.log(sqINFO, "Start BLE service");
  // 初始化 BLE 服务
  SQ_BLEService.init(30000);
  // 创建 BLE 服务的 FreeRTOS 任务
  xTaskCreate(SQ_BLEServiceClass::bleTaskWrapper, "bleTask", 4096, NULL, 1, NULL);

#endif
}

esp_sleep_wakeup_cause_t print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }

  return wakeup_reason;
}

uint64_t get_chip_id()
{
  uint64_t chipid = ESP.getEfuseMac();
  return chipid;
}