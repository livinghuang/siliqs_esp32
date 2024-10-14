#include "siliqs_heltec_esp32.h"

FileSystem fileSystem; // Create an instance of FileSystem

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
  Serial.println("初始化 NimBLE 服务...");
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
  Serial.println("NimBLE 服务初始化完成");
}
#endif

void siliqs_heltec_esp32_setup(int print_level)
{
  Serial.begin(115200);
  console.begin(print_level);

  // 初始化文件系统
  setupFileSystem();

  fileSystem.writeFile("/test.txt", "Hello, SiliQ!");
#ifdef USE_NIMBLE
  start_nimble_service();
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