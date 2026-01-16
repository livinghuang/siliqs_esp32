#include "bsp.h"
#include "serial_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

SerialConsole console; // 全局的 SerialConsole 對象

// 構造函數，初始化串行通信
SerialConsole::SerialConsole()
{
}

void SerialConsole::begin(int level)
{
  print_level = level;
}
// 處理接收到的數據
bool SerialConsole::handleToRadio(const uint8_t *buf, size_t len)
{
  // 此處編寫你的邏輯
  return true;
}

// 覆蓋 write 函數，處理單字節輸出
size_t SerialConsole::write(uint8_t c)
{
  if (c == '\n')
  {
    Serial.write('\r'); // 在換行符之前添加回車符
  }
  return Serial.write(c);
}

// 根據打印級別和日誌級別決定是否輸出日誌
bool should_log(Siliq_Log_Record_Level logLevel, int print_level)
{
  switch (print_level)
  {
  case SQ_NONE:
    return false; // 不打印任何日誌
  case SQ_INFO:
    return (logLevel == sqINFO || logLevel == sqERROR); // 打印 when Siliq_Log_Record_Level = INFO,ERROR but DEBUG,WARNING,CRITICAL,不打印
  case SQ_DEBUG:
    return true; // 打印所有日誌
  default:
    return false;
  }
}

// 將日誌輸出到串行端口
void SerialConsole::log_to_serial(Siliq_Log_Record_Level logLevel, const char *format, va_list arg)
{
  if (!should_log(logLevel, print_level))
  {
    return; // 如果當前級別不應該被打印，直接返回
  }
  // 獲取當前任務名稱
  const char *taskName = pcTaskGetName(NULL);

  // 打印日誌級別
  Serial.print("[");
  switch (logLevel)
  {
  case sqDEBUG:
    Serial.print("DEBUG");
    break;
  case sqINFO:
    Serial.print("INFO");
    break;
  case sqWARN:
    Serial.print("WARNING");
    break;
  case sqERROR:
    Serial.print("ERROR");
    break;
  case sqCRITICAL:
    Serial.print("CRITICAL");
    break;
  default:
    Serial.print("UNSET");
    break;
  }
  Serial.print("] ");

  // 如果有任務名稱且不是 "loopTask"，則打印任務名稱
  if (taskName && strcmp(taskName, "loopTask") != 0)
  {
    Serial.print("[Task: ");
    Serial.print(taskName);
    Serial.print("] ");
  }

  // 打印格式化的日誌消息
  char logBuffer[256];
  vsnprintf(logBuffer, sizeof(logBuffer), format, arg);
  Serial.println(logBuffer);
}

// 重載 log 函數以接受 Arduino String 類型的輸入
void SerialConsole::log(Siliq_Log_Record_Level logLevel, const String &message)
{
  log(logLevel, message.c_str()); // 將 String 轉換為 C 字符串並傳遞到主 log 函數
}
// 重載 log 函數以接受字節數組並以十六進制格式打印
// 重載 log 函數以接受字節數組並以十六進制格式打印
void SerialConsole::log(Siliq_Log_Record_Level logLevel, const uint8_t *buffer, size_t length)
{
  const size_t CHUNK_SIZE = 512; // 每次最多處理 512 字節
  char message[CHUNK_SIZE];      // 固定大小的緩衝區

  size_t bytesProcessed = 0;
  while (bytesProcessed < length)
  {
    size_t chunkLength = min(length - bytesProcessed, CHUNK_SIZE / 3); // 每個字節占 3 個字符（兩位十六進制加一個空格）

    size_t index = 0;
    for (size_t i = 0; i < chunkLength; i++)
    {
      if (buffer[bytesProcessed + i] < 0x10)
      {
        message[index++] = '0'; // 保持兩位數的顯示格式
      }
      index += sprintf(&message[index], "%X ", buffer[bytesProcessed + i]); // 將字節轉換為十六進制
    }
    message[index] = '\0'; // 確保字符串以 '\0' 結尾

    // 調用原始的 log 函數打印當前分段
    log(logLevel, message);

    // 更新已處理的字節數
    bytesProcessed += chunkLength;
  }
}
// 用於變長參數的日誌記錄輔助函數
void SerialConsole::log(Siliq_Log_Record_Level logLevel, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);

  // 調用 log_to_serial 來記錄日誌
  log_to_serial(logLevel, format, arg);
  va_end(arg);
}

// 清空串行緩衝區
void SerialConsole::flush()
{
  Serial.flush();
}