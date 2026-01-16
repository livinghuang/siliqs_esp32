#include "systime.h"

#include "esp_sleep.h"
#include "esp_timer.h"
#include <sys/time.h>

// RTC 記憶體儲存：進入 sleep 前的時間與 esp_timer（微秒）
RTC_DATA_ATTR time_t saved_rtc_time = 0;
RTC_DATA_ATTR int64_t saved_esp_us = 0;

SysTime systemTime;

bool SysTime::begin()
{
  restoreFromRTC();
  return true;
}

bool SysTime::begin(time_t t)
{
  if (!restoreFromRTC())
  {
    set(t);
  }
  return true;
}

bool SysTime::begin(const struct tm &t)
{
  // mktime 可能會修改 tm（例如補齊 wday/yday），所以用 copy 保險
  struct tm tmp = t;
  time_t tt = mktime(&tmp);
  return begin(tt);
}

bool SysTime::restoreFromRTC()
{
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER && saved_rtc_time > 0)
  {
    struct timeval tv = {.tv_sec = saved_rtc_time, .tv_usec = 0};
    settimeofday(&tv, nullptr);
    return true;
  }
  return false;
}

void SysTime::set(time_t t)
{
  struct timeval tv = {.tv_sec = t, .tv_usec = 0};
  settimeofday(&tv, nullptr);
}

void SysTime::set(const uint8_t *buf)
{
  // buf 必須至少 8 bytes
  uint64_t t =
      ((uint64_t)buf[0] << 56) |
      ((uint64_t)buf[1] << 48) |
      ((uint64_t)buf[2] << 40) |
      ((uint64_t)buf[3] << 32) |
      ((uint64_t)buf[4] << 24) |
      ((uint64_t)buf[5] << 16) |
      ((uint64_t)buf[6] << 8) |
      ((uint64_t)buf[7]);
  set((time_t)t);
}

time_t SysTime::now() const
{
  time_t t;
  time(&t);
  return t;
}

void SysTime::saveToRTC()
{
  saved_rtc_time = now();
  saved_esp_us = esp_timer_get_time();
}

void SysTime::print() const
{
  time_t t = now();
  Serial.printf("UNIX: %lu\n", (unsigned long)t);

  struct tm ti;
  if (getLocalTime(&ti))
  {
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
    Serial.println(buf);
  }
}

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
