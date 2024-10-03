#pragma once

#include "Arduino.h" // 用於串行通信
#include <stdarg.h>

enum Siliq_DEBUG_INFO_PRINT_LEVEL
{
  SQ_NONE, // 不打印任何信息
  SQ_INFO, // 打印 when Siliq_Log_Record_Level = INFO,ERROR but DEBUG,WARNING,CRITICAL,不打印
  SQ_DEBUG // 打印 when Siliq_Log_Record_Level = ALL
};

enum Siliq_Log_Record_Level
{
  sqUNSET,
  sqDEBUG,
  sqINFO,
  sqWARNING,
  sqERROR,
  sqCRITICAL
};

class SerialConsole
{
  bool usingProtobufs = false; // 是否使用 protobuf 進行日誌記錄

public:
  SerialConsole(); // 構造函數

  void begin(int level = SQ_NONE);

  // 處理接收到的數據並處理它
  bool handleToRadio(const uint8_t *buf, size_t len);

  // 覆蓋 write 函數
  size_t write(uint8_t c);

  // 用於將日誌輸出到串行端口的函數
  void log_to_serial(Siliq_Log_Record_Level logLevel, const char *format, va_list arg);

  // 重載 log 函數以支持 Arduino String 類型
  void log(Siliq_Log_Record_Level logLevel, const String &message);

  // 重載 log 函數以支持字節數組的打印
  void log(Siliq_Log_Record_Level logLevel, const uint8_t *buffer, size_t length);

  // 用於變長參數的日誌記錄輔助函數
  void log(Siliq_Log_Record_Level logLevel, const char *format, ...);

  // 清空串行緩衝區
  void flush();

private:
  int print_level;
};

extern SerialConsole console;