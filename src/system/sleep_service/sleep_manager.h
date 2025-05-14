// sleep_manager.h
#pragma once
#include <Arduino.h>
#include <esp_sleep.h>
#include "system/systime_service/systime.h"
#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define MIN_SLEEP_TIME_MS 100
// RTC fast memory 中保留的跨深眠時間戳（秒）
extern RTC_DATA_ATTR time_t rtcSleepStartTime;
enum WakeUpLevel
{
  WAKE_HIGH,
  WAKE_LOW
};

class SleepManager
{
public:
  // 開機後的 millis 時間戳（毫秒）
  uint32_t msSleepStartTime = 0;
  SleepManager()
  {
    startSleepCycle();
  }
  // 設定 sleep cycle 週期（單位：ms）
  void set_sleep_cycle_time(uint32_t ms)
  {
    sleep_cycle_time_ms = ms;
  }

  // 開始一個新的週期，記錄目前時間（支援 millis 與 RTC）
  void startSleepCycle()
  {
    rtcSleepStartTime = systemTime.get_time();
    msSleepStartTime = millis();
  }

  // 設定 GPIO 為喚醒來源（支援 HIGH / LOW）
  void set_gpio_to_wake_up(int pin, WakeUpLevel level)
  {
    gpio_reset_pin((gpio_num_t)pin);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_INPUT);

    if (level == WAKE_HIGH)
    {
      gpio_pulldown_en((gpio_num_t)pin);
      gpio_pullup_dis((gpio_num_t)pin);
      esp_deep_sleep_enable_gpio_wakeup((1ULL << pin), ESP_GPIO_WAKEUP_GPIO_HIGH);
    }
    else
    {
      gpio_pulldown_dis((gpio_num_t)pin);
      gpio_pullup_en((gpio_num_t)pin);
      esp_deep_sleep_enable_gpio_wakeup((1ULL << pin), ESP_GPIO_WAKEUP_GPIO_LOW);
    }
  }

  // 執行 sleep，會根據週期剩餘時間決定 sleep 時間
  void goToSleepByCycle()
  {
    uint32_t used_ms = 0;

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
    {
      // deep sleep 醒來，用 RTC 來推算經過時間
      time_t now = systemTime.get_time();
      used_ms = (now - rtcSleepStartTime) * 1000;
    }
    else
    {
      // 非 deep sleep，直接用 millis
      used_ms = millis() - msSleepStartTime;
    }

    uint32_t sleep_ms = MIN_SLEEP_TIME_MS;
    if (used_ms >= sleep_cycle_time_ms)
    {
      Serial.println("[SleepManager] used time > cycle, sleep minimum time");
    }
    else
    {
      sleep_ms = sleep_cycle_time_ms - used_ms;
    }

    Serial.printf("[SleepManager] sleeping for %u ms...\n", sleep_ms);
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_ms * 1000); // 轉成 μs
    esp_deep_sleep_start();
  }

private:
  uint32_t sleep_cycle_time_ms = 10000; // 預設 10 秒（ms）
};

extern SleepManager SysSleep;
void test_sleep();
#endif