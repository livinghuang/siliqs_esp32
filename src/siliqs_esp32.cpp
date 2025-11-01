#include "siliqs_esp32.h"

FileSystem fileSystem; // Create an instance of FileSystem
RTC_DATA_ATTR uint32_t bootCount = 0;
void setupFileSystem()
{
  if (fileSystem.begin())
  {
    console.log(sqINFO, "file system intialized！");
  }
  else
  {
    console.log(sqINFO, "file system initialization failed");
  }
}

#ifdef USE_NIMBLE
void start_nimble_service(void)
{
  console.log(sqINFO, "初始化 NimBLE 服务...");
  nimbleService.init();

  // 创建一个 FreeRTOS 任务来处理 BLE 扫描
  xTaskCreate(
      SQNimBLEService::bleTaskWrapper, // 任务函数包装器
      "NimBLE Scan Task",              // 任务名称
      4096,                            // 堆栈大小（字节）
      &nimbleService,                  // 传递给任务的参数（NimBLE 服务实例）
      1,                               // 任务优先级
      NULL                             // 任务句柄（可以为 NULL）
  );
  console.log(sqINFO, "NimBLE 服务初始化完成");
}
#endif

#ifdef USE_EXTERNAL_XTAL
RTC_DATA_ATTR uint32_t cal_32k = 0;
void external_32k_setup()
{
  console.log(sqINFO, "Testing External 32.768 kHz Crystal...");

  if (cal_32k == 0)
  {
    // First boot or calibration missing, initialize and calibrate the crystal
    rtc_clk_32k_bootstrap(512);
    rtc_clk_32k_enable(true);

    if (!rtc_clk_32k_enabled())
    {
      console.log(sqERROR, "32.768 kHz crystal not enabled. Falling back to internal clock.");
      return;
    }

    console.log(sqINFO, "32.768 kHz crystal enabled successfully.");

    // Perform calibration
    cal_32k = rtc_clk_cal(RTC_CAL_32K_XTAL, 1000);
    if (cal_32k == 0)
    {
      console.log(sqINFO, "Calibration failed: 32.768 kHz crystal is not stable.");
    }
    else
    {
      console.log(sqINFO, "Calibration successful.");
      float frequency = (1 << 19) * 1000.0f / cal_32k;
      console.log(sqINFO, "Calibrated frequency: %.3f kHz\n", frequency);
    }
  }
  else
  {
    // Use the saved calibration value
    console.log(sqINFO, "Using saved calibration value.");
    float frequency = (1 << 19) * 1000.0f / cal_32k;
    console.log(sqINFO, "Calibrated frequency (saved): %.3f kHz\n", frequency);
  }

  // Set the RTC slow clock source to the external crystal
  rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
  rtc_slow_freq_t slow_clk = rtc_clk_slow_freq_get();

  if (slow_clk == RTC_SLOW_FREQ_32K_XTAL)
  {
    console.log(sqINFO, "RTC slow clock source set to external 32.768 kHz crystal.");
  }
  else
  {
    console.log(sqINFO, "Failed to set RTC slow clock source to external crystal.");
  }
}
#endif

SemaphoreHandle_t i2cMutex = nullptr;
void siliqs_esp32_setup(int print_level, int serial_speed)
{
  Serial.begin(serial_speed);
  console.begin(print_level);
  bootCount++;
  console.log(sqINFO, "Boot number: %d", bootCount);
  // 初始化文件系统
  setupFileSystem();

  fileSystem.writeFile("/test.txt", "Hello, SiliQ!");
#ifdef USE_NIMBLE
  start_nimble_service();
#endif

#ifdef USE_EXTERNAL_XTAL
  external_32k_setup();
#endif

  i2cMutex = xSemaphoreCreateMutex();
  if (i2cMutex == nullptr)
  {
    console.log(sqINFO, "创建 i2cMutex 互斥锁失败！");
  }
}

esp_sleep_wakeup_cause_t print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    console.log(sqINFO, "Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    console.log(sqINFO, "Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    console.log(sqINFO, "Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    console.log(sqINFO, "Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    console.log(sqINFO, "Wakeup caused by ULP program");
    break;
  default:
    console.log(sqINFO, "Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }

  return wakeup_reason;
}

uint64_t get_chip_id()
{
  uint64_t chipid = ESP.getEfuseMac();
  return chipid;
}

// Function to store system data as a Base64 string in a file
bool storageSystemData(void *global_system_data, size_t dataSize)
{
  // Convert struct to a byte array
  uint8_t *data = (uint8_t *)global_system_data;

  // Base64 encode the byte array
  size_t encodedLength = Base64::encodedLength(dataSize);
  char encodedData[encodedLength + 1]; // +1 for null terminator
  Base64::encode(encodedData, (char *)data, dataSize);

  // Write the encoded data to the file
  if (fileSystem.writeFile("/system.txt", encodedData))
  {
    console.log(sqINFO, "System data saved successfully");
    return true;
  }
  else
  {
    console.log(sqERROR, "Failed to save system data");
    return false;
  }
}

// Function to read system data from a file and decode it
bool readSystemData(void *global_system_data, size_t dataSize)
{
  // Read the encoded Base64 string from the file
  String systemDataString = fileSystem.readFile("/system.txt");
  if (systemDataString.isEmpty())
  {
    console.log(sqERROR, "Failed to read system data");
    return false;
  }

  // Base64 decode the string back to the byte array
  size_t decodedLength = Base64::decodedLength(systemDataString.c_str());
  if (decodedLength != dataSize)
  {
    console.log(sqERROR, "Decoded data size mismatch");
    return false;
  }

  uint8_t decodedData[decodedLength];
  Base64::decode((char *)decodedData, systemDataString.c_str(), systemDataString.length());

  // Copy the decoded data back into the global_system_data struct
  memcpy(global_system_data, decodedData, dataSize);

  console.log(sqINFO, "System data loaded successfully");
  return true;
}
void gotoSleep(uint32_t ms)
{
  if (ms == 0)
  {
    return;
  }
  esp_sleep_enable_timer_wakeup(ms * 1000); // function uses uS
  console.log(sqINFO, "Sleeping... will wake in " + String(ms) + " milliseconds");
  Serial.flush();
  esp_deep_sleep_start();
  // if this appears in the serial debug, we didn't go to sleep!
  // so take defensive action
  Serial.println(F("\n\n### Sleep failed, delay of 15 seconds & then restart ###\n"));
  delay(15000);
  ESP.restart();
}
