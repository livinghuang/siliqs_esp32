#pragma once
#include "siliqs_esp32.h"
#include <Arduino.h>
#include <functional>
#include "esp_partition.h"
#include <algorithm>
// 错误代码定义
#define UPDATE_ERROR_OK (0)
#define UPDATE_ERROR_WRITE (1)
#define UPDATE_ERROR_ERASE (2)
#define UPDATE_ERROR_READ (3)
#define UPDATE_ERROR_SPACE (4)
#define UPDATE_ERROR_SIZE (5)
#define UPDATE_ERROR_STREAM (6)
#define UPDATE_ERROR_MAGIC_BYTE (7)
#define UPDATE_ERROR_ACTIVATE (8)
#define UPDATE_ERROR_NO_PARTITION (9)
#define UPDATE_ERROR_BAD_ARGUMENT (10)
#define UPDATE_ERROR_ABORT (11)

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

#define U_FLASH 0
#define U_SPIFFS 100
#define SPI_FLASH_SEC_SIZE 4096  /**< SPI Flash sector size */
#define SPI_SECTORS_PER_BLOCK 16 // 通常较大的擦除块为32k/64k
#define SPI_FLASH_BLOCK_SIZE (SPI_SECTORS_PER_BLOCK * SPI_FLASH_SEC_SIZE)

class SQUpdateClass
{
public:
  typedef std::function<void(size_t, size_t)> THandlerFunction_Progress;

  SQUpdateClass();

  // 设置进度回调
  SQUpdateClass &onProgress(THandlerFunction_Progress fn);

  // 开始OTA更新
  bool begin();

  // 写入数据到闪存
  size_t write(uint8_t *data, size_t len);
  size_t write4096(uint8_t *data, size_t len, bool isFinalPacket);
  size_t write512(uint8_t *data, size_t len, bool isFinalPacket);
  size_t writebychunksize(uint8_t *data, size_t len, size_t chunkSize, bool isFinalPacket);
  // bool flushBuffer();
  void setUpdateSize(size_t size);

  // 完成OTA更新
  bool end();

  // 获取最后的错误信息
  // uint8_t getError();
  // void clearError();
  // bool hasError();
  // bool isRunning();
  // bool isFinished();
  // size_t size();
  size_t progress();
  size_t remaining();

  // 获取错误的字符串描述
  const char *errorString();
  // 辅助函数，执行更新所需的其他操作
  template <typename T>
  size_t write(T &data);

private:
  void _reset();
  void _abort(uint8_t err);
  bool _writeBuffer();
  bool _verifyHeader(uint8_t data);
  bool _verifyEnd();
  bool _enablePartition(const esp_partition_t *partition);

  uint8_t _error;
  uint8_t *_buffer;
  uint8_t *_skipBuffer;
  size_t _bufferLen;
  size_t _size;
  THandlerFunction_Progress _progress_callback;
  uint32_t _progress;
  uint32_t _command;
  const esp_partition_t *_partition;
};

extern SQUpdateClass sqUpdate;
