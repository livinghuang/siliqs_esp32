#include "siliqs_heltec_esp32.h"
void siliqs_heltec_esp32_setup(void)
{
  Serial.begin(115200);
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
  Serial.printf("ESP32ChipID=%04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  return chipid;
}

// void print_bytes(const uint8_t *data, int length)
// {
//   for (int i = 0; i < length; i++)
//   {
//     // Print each byte in hexadecimal format with leading zeros
//     if (data[i] < 0x10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(data[i], HEX);
//     Serial.print(" ");
//   }
//   Serial.println(); // Print a newline character at the end
// }
// void print_bytes_reverse(uint8_t *data, int length)
// {
//   for (int i = length - 1; i >= 0; i--)
//   {
//     if (data[i] < 0x10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(data[i], HEX);
//     // Serial.print(" ");
//   }
//   Serial.println();
// }