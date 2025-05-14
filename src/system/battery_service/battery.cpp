#include "bsp.h"
#include "battery.h"
#ifdef USE_BATTERY_SERVICE
battery batt(false, Serial);

#define SERIAL1_TXD 18
#define SERIAL1_RXD 19
// #define BATTERY_TEST_OFFSET 0.6417
// #define BATTERY_TEST_GAIN 0.8333
#define BATTERY_TEST_OFFSET 0.6084
#define BATTERY_TEST_GAIN 0.8451
// Vreal = 0.8451 * Vadc + 0.6084;
//  #define BATTERY_OFFSET 0.25
//  #define BATTERY_GAIN 0.945

/*
BatteryService - 電池服務類別

此服務會在背景中以任務 (task) 方式運行，用來提供電池電壓資訊。
請勿直接使用或修改這個類別。

---

校正電池電壓讀值的使用方式：

Step 1：
設定初始值
  OFFSET = 0
  GAIN = 1.0

Step 2：
分別提供兩組已知實際電壓條件進行測試：
  - 輸入電壓為 3.0V，記錄此時 ADC 所讀取的電壓 Vadc1
  - 輸入電壓為 4.2V，記錄此時 ADC 所讀取的電壓 Vadc2

Step 3：
計算校正參數（線性校正）

  GAIN = (Vreal2 - Vreal1) / (Vadc2 - Vadc1)
       = (4.2 - 3.0) / (Vadc2 - Vadc1)

  OFFSET = Vreal1 - (GAIN * Vadc1)
         = 3.0 - (GAIN * Vadc1)

  之後所有電壓讀值可透過：
    Vreal = GAIN * Vadc + OFFSET
  進行校正補償
  example
  Vreal = 0.8333 * Vadc + 0.6417;

*/

void test_battery(void)
{
  // HardwareSerial MySerial1(1);
  // class battery batt(false, MySerial1);
  // MySerial1.begin(115200, SERIAL_8N1, SERIAL1_RXD, SERIAL1_TXD);
  Serial.begin(115200);
  batt.set_correction(BATTERY_TEST_GAIN, BATTERY_TEST_OFFSET);
  batt.begin(1, 15); // 背景中以任務 (task) 方式運行

  while (true)
  {
    Serial.println("[Battery Test] Current battery data:");
    batt.print();
    delay(1000); // 每 1 秒印一次
  }
}
#endif