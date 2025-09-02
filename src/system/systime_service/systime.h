#pragma once
#include "Arduino.h"
#include <time.h>
#include "esp_sleep.h"
#include "esp_system.h"

// 在 RTC FAST memory 中保留時間資料（重開機時失效，深眠不會）
extern time_t saved_rtc_time;
extern int64_t saved_esp_us;

class systime
{
public:
  void begin(struct tm t)
  {
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
    {
      // 從 RTC 記憶體還原
      struct timeval tv = {.tv_sec = saved_rtc_time, .tv_usec = 0};
      settimeofday(&tv, nullptr);
      Serial.printf("Restored time from RTC: %lu\n", (unsigned long)saved_rtc_time);
    }
    else
    {
      // 冷開機設定時間
      time_t localTime = mktime(&t);
      set_time(localTime);
      Serial.printf("Cold boot - default time set: %lu\n", (unsigned long)localTime);
    }
  }

  void set_time(time_t t)
  {
    struct timeval tv = {.tv_sec = t, .tv_usec = 0};
    settimeofday(&tv, nullptr);
  }

  void set_time(uint8_t *time_buffer)
  {
    uint64_t local_time =
        ((uint64_t)time_buffer[0] << 56) |
        ((uint64_t)time_buffer[1] << 48) |
        ((uint64_t)time_buffer[2] << 40) |
        ((uint64_t)time_buffer[3] << 32) |
        ((uint64_t)time_buffer[4] << 24) |
        ((uint64_t)time_buffer[5] << 16) |
        ((uint64_t)time_buffer[6] << 8) |
        ((uint64_t)time_buffer[7]);
    set_time(local_time);
  }

  time_t get_time()
  {
    time_t now;
    time(&now);
    return now;
  }

  void reserve_to_memory()
  {
    saved_rtc_time = get_time();
    saved_esp_us = esp_timer_get_time(); // optional, for future use
    Serial.printf("Saved time to RTC memory: %lu\n", (unsigned long)saved_rtc_time);
  }

  void print()
  {
    time_t now = get_time();
    Serial.print("UNIX Timestamp: ");
    Serial.println((unsigned long)now);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      char buffer[30];
      strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.print("Formatted Time: ");
      Serial.println(buffer);
    }
    else
    {
      Serial.println("Failed to get local time.");
    }
  }
};

extern systime systemTime;
void test_systime();
