#include "systime.h"

systime systemTime;

// RTC 記憶體儲存：進入 sleep 前的時間與 esp_timer（微秒）
RTC_DATA_ATTR time_t saved_rtc_time = 0;
RTC_DATA_ATTR int64_t saved_esp_us = 0;

void test_systime()
{
  Serial.begin(115200);
  delay(1000);
  struct tm t;
  t.tm_year = 2025 - 1900; // 年份從 1900 開始計算
  t.tm_mon = 2;            // 0-based，0 是 1 月，2 是 3 月
  t.tm_mday = 22;          // 日
  t.tm_hour = 12;
  t.tm_min = 0;
  t.tm_sec = 0;

  systemTime.begin(t);
  // 顯示目前時間，並儲存進 RTC 記憶體
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    Serial.println("Time is available");
    systemTime.reserve_to_memory(); // 儲存目前時間與 esp_timer
  }

  // 顯示時間 4 次
  for (uint8_t c = 0; c <= 3; ++c)
  {
    delay(1000);
    Serial.printf("Current time: %lu\n", (unsigned long)systemTime.get_time());
    systemTime.print();
  }

  // 設定下一次喚醒時間並進入 deep sleep
  uint32_t deep_sleep_sec = 6;
  Serial.println("Going to sleep for " + String(deep_sleep_sec) + " seconds...");
  esp_sleep_enable_timer_wakeup(deep_sleep_sec * 1000000); // 6 秒
  time_t now = systemTime.get_time();
  systemTime.set_time(now + deep_sleep_sec);
  systemTime.reserve_to_memory(); // 睡前儲存時間
  esp_deep_sleep_start();
}
