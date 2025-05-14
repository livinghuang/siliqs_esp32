#include "sleep_manager.h"
#if defined(CONFIG_IDF_TARGET_ESP32C3)
SleepManager SysSleep;
RTC_DATA_ATTR time_t rtcSleepStartTime = 0;
void test_sleep()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("== SleepManager Test ==");

  // 設定 sleep 週期為 10 秒
  SysSleep.set_sleep_cycle_time(10000);

  // 記錄這次週期的開始時間
  // SysSleep.startSleepCycle();

  // 顯示目前 RTC 時間
  time_t now = systemTime.get_time();
  Serial.printf("Current RTC time: %lu\n", now);

  // 設定 GPIO 0 為喚醒來源（低電平喚醒）, not every pin could wake up system in deep sleep
  SysSleep.set_gpio_to_wake_up(0, WAKE_LOW);

  // 模擬工作耗時
  delay(2500); // 模擬處理耗時 2.5 秒

  // 進入深眠（根據週期自動補足時間）
  SysSleep.goToSleepByCycle();
}
#endif