#include "SQUpdate.h"
#include <Arduino.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_log.h>

// 默认构造函数
SQUpdateClass::SQUpdateClass()
    : _error(UPDATE_ERROR_OK),
      _buffer(nullptr),
      _skipBuffer(nullptr),
      _bufferLen(0),
      _size(0),
      _progress(0),
      _partition(nullptr),
      _progress_callback(nullptr) {}

// 设置进度回调函数
SQUpdateClass &SQUpdateClass::onProgress(THandlerFunction_Progress fn)
{
  _progress_callback = fn;
  return *this;
}

// 开始OTA更新
bool SQUpdateClass::begin()
{
  _reset();
  _error = UPDATE_ERROR_OK;

  _partition = esp_ota_get_next_update_partition(nullptr);
  console.log(sqINFO, "Update partition: %s", _partition->label);
  console.log(sqINFO, "Update partition size: %d", _partition->size);
  if (!_partition)
  {
    console.log(sqINFO, "Update partition not found");
    return false;
  }

  _buffer = new uint8_t[SPI_FLASH_BLOCK_SIZE];
  if (!_buffer)
  {
    console.log(sqINFO, "Failed to allocate buffer");
    return false;
  }
  return true;
}

// 重置内部状态
void SQUpdateClass::_reset()
{
  if (_buffer)
  {
    delete[] _buffer;
    _buffer = nullptr;
  }
  if (_skipBuffer)
  {
    delete[] _skipBuffer;
    _skipBuffer = nullptr;
  }
  _bufferLen = 0;
  _progress = 0;
  _size = 0;
}

// 写入固件数据
size_t SQUpdateClass::write(uint8_t *data, size_t len)
{
  if (_error != UPDATE_ERROR_OK || !_partition || len == 0)
  {
    return 0;
  }

  size_t left = len;
  while (left > 0)
  {
    size_t to_write = std::min<size_t>(SPI_FLASH_BLOCK_SIZE, left); // Explicitly use std::min
    console.log(sqINFO, "Writing %d bytes, %d bytes remaining", to_write, left - to_write);
    memcpy(_buffer, data + (len - left), to_write);
    _bufferLen = to_write; // 設置緩衝區長度

    if (!_writeBuffer())
    {
      return len - left;
    }
    left -= to_write;
  }

  return len;
}

size_t SQUpdateClass::write4096(uint8_t *data, size_t len, bool isFinalPacket)
{
  constexpr size_t BLOCK_SIZE = SPI_FLASH_SEC_SIZE;

  if (_error != UPDATE_ERROR_OK || !_partition)
  {
    console.log(sqERROR, "Write failed: Invalid state or partition.");
    return 0;
  }

  // Log the input data
  console.log(sqINFO, "Writing 4 KB block to flash memory, Final Packet: %s", isFinalPacket ? "Yes" : "No");

  // Calculate aligned offset for erase operation
  size_t erase_offset = _progress & ~(BLOCK_SIZE - 1);
  console.log(sqINFO, "Erasing 4 KB block at offset 0x%08X", erase_offset);

  // Erase the necessary flash block
  esp_err_t erase_result = esp_partition_erase_range(_partition, erase_offset, BLOCK_SIZE);
  if (erase_result != ESP_OK)
  {
    console.log(sqERROR, "Failed to erase 4 KB block at offset 0x%08X, Error: %s", erase_offset, esp_err_to_name(erase_result));
    _error = UPDATE_ERROR_ERASE;
    return 0;
  }

  // Write the block to flash memory
  // size_t write_size = isFinalPacket ? std::min<size_t>(BLOCK_SIZE, _size - _progress) : BLOCK_SIZE;
  size_t write_size = len;
  if ((!isFinalPacket) && (len != BLOCK_SIZE))
  {
    console.log(sqERROR, "Write failed: Invalid size for non-final packet.");
    return 0;
  }
  console.log(sqINFO, "Writing %d bytes to offset 0x%08X", write_size, _progress);

  esp_err_t write_result = esp_partition_write(_partition, _progress, data, write_size);
  if (write_result != ESP_OK)
  {
    console.log(sqERROR, "Failed to write block at offset 0x%08X, Error: %s", _progress, esp_err_to_name(write_result));
    _error = UPDATE_ERROR_WRITE;
    return 0;
  }

  // Update progress
  _progress += write_size;
  console.log(sqINFO, "Written 4096 Done");
  return write_size;
}
size_t SQUpdateClass::writebychunksize(uint8_t *data, size_t len, size_t chunkSize, bool isFinalPacket)
{
  size_t result = len;
  constexpr size_t BLOCK_SIZE = SPI_FLASH_SEC_SIZE; // 4 KB

  // Validate state and partition
  if (_error != UPDATE_ERROR_OK || !_partition)
  {
    console.log(sqERROR, "Write failed: Invalid state or partition.");
    return 0;
  }

  // Static buffer and offset
  static uint8_t buffer[BLOCK_SIZE];
  static size_t buffer_offset = 0;

  // Check input data validity
  if (data == nullptr)
  {
    console.log(sqERROR, "Input data is null.");
    return 0;
  }

  // Validate chunk size
  if (len > chunkSize)
  {
    console.log(sqERROR, "Input data length exceeds chunk size.");
    return 0;
  }

  // Add data to buffer
  memcpy(buffer + buffer_offset, data, len);
  buffer_offset += len;

  console.log(sqINFO, "Added %d bytes to buffer. Current buffer offset: %d", len, buffer_offset);

  // Flush buffer if full or if it's the final packet
  if ((buffer_offset >= BLOCK_SIZE) || isFinalPacket)
  {
    console.log(sqINFO, "Buffer full or final packet. Writing %d bytes to flash.", buffer_offset);

    size_t written = write4096(buffer, buffer_offset, isFinalPacket);

    if (written != buffer_offset)
    {
      console.log(sqERROR, "Failed to write block");
      _error = UPDATE_ERROR_WRITE;
      return 0;
    }

    if (isFinalPacket)
    {
      console.log(sqINFO, "Final packet written. All data successfully written to flash.");
    }

    buffer_offset = 0; // Reset buffer offset
  }

  return result;
}

bool SQUpdateClass::end()
{
  if (_error != UPDATE_ERROR_OK)
  {
    console.log(sqERROR, "End failed : Invalid state or size.%ld");
    return false;
  }
  console.log(sqINFO, "Activating new OTA partition...");
  esp_err_t err = esp_ota_set_boot_partition(_partition);
  return true;
}

// 写入缓冲区到闪存
bool SQUpdateClass::_writeBuffer()
{
  console.log(sqINFO, "Current progress: %d", _progress);
  console.log(sqINFO, "Buffer length to write: %d", _bufferLen);
  // 偏移量应从分区起始地址算起
  size_t offset = _progress; // 以分区内的偏移量为基准

  // 输出分区信息
  console.log(sqINFO, "Partition Information:");
  console.log(sqINFO, "Label: %s", _partition->label);
  console.log(sqINFO, "Type: 0x%02X", _partition->type);
  console.log(sqINFO, "Subtype: 0x%02X", _partition->subtype);
  console.log(sqINFO, "Address: 0x%08X", _partition->address);
  console.log(sqINFO, "Size: %d bytes", _partition->size);
  console.log(sqINFO, "Erase Size: %d bytes", SPI_FLASH_BLOCK_SIZE);

  // 执行擦除
  console.log(sqINFO, "Attempting to erase partition range: Offset = 0x%08X, Size = %d", offset, SPI_FLASH_BLOCK_SIZE);
  esp_err_t eraseResult = esp_partition_erase_range(_partition, offset, SPI_FLASH_BLOCK_SIZE);
  if (eraseResult != ESP_OK)
  {
    console.log(sqERROR, "Partition erase failed at offset 0x%08X, size %d. Error code: %d (%s)",
                offset, SPI_FLASH_BLOCK_SIZE, eraseResult, esp_err_to_name(eraseResult));
    _error = UPDATE_ERROR_ERASE;
    return false;
  }

  // 执行写入
  console.log(sqINFO, "Partition erase successful. Writing buffer to offset: 0x%08X", offset);
  esp_err_t writeResult = esp_partition_write(_partition, offset, _buffer, _bufferLen);
  if (writeResult != ESP_OK)
  {
    console.log(sqERROR, "Partition write failed at offset 0x%08X, size %d. Error code: %d (%s)",
                offset, _bufferLen, writeResult, esp_err_to_name(writeResult));
    _error = UPDATE_ERROR_WRITE;
    return false;
  }

  // 更新进度
  console.log(sqINFO, "Buffer written successfully to offset: 0x%08X, Size: %d", offset, _bufferLen);
  _progress += _bufferLen;
  _bufferLen = 0;

  return true;
}

// 获取剩余空间
size_t SQUpdateClass::remaining()
{
  return _size - _progress;
}

// 获取当前进度
size_t SQUpdateClass::progress()
{
  return _progress;
}

// 获取错误的字符串描述
const char *SQUpdateClass::errorString()
{
  switch (_error)
  {
  case UPDATE_ERROR_OK:
    return "No Error";
  case UPDATE_ERROR_WRITE:
    return "Flash Write Failed";
  case UPDATE_ERROR_ERASE:
    return "Flash Erase Failed";
  case UPDATE_ERROR_READ:
    return "Flash Read Failed";
  case UPDATE_ERROR_SPACE:
    return "Not Enough Space";
  case UPDATE_ERROR_SIZE:
    return "Bad Size Given";
  case UPDATE_ERROR_STREAM:
    return "Stream Read Timeout";
  case UPDATE_ERROR_MAGIC_BYTE:
    return "Wrong Magic Byte";
  case UPDATE_ERROR_ACTIVATE:
    return "Could Not Activate The Firmware";
  case UPDATE_ERROR_NO_PARTITION:
    return "Partition Could Not be Found";
  case UPDATE_ERROR_BAD_ARGUMENT:
    return "Bad Argument";
  case UPDATE_ERROR_ABORT:
    return "Aborted";
  default:
    return "UNKNOWN";
  }
}

// 写入缓冲区中的数据
template <typename T>
size_t SQUpdateClass::write(T &data)
{
  size_t written = 0;
  size_t available = data.available();

  while (available)
  {
    size_t to_buff = std::min<size_t>(SPI_FLASH_BLOCK_SIZE, remaining()); // Explicitly use std::min
    size_t read_size = data.readBytes(_buffer, to_buff);
    written += read_size;

    if (!_writeBuffer())
    {
      return written;
    }

    available = data.available();
  }
  return written;
}

void SQUpdateClass::setUpdateSize(size_t size)
{
  _size = size;
}

void SQUpdateClass::_abort(uint8_t err)
{
  _error = err;
  _reset(); // 重置内部状态
}
bool SQUpdateClass::_enablePartition(const esp_partition_t *partition)
{
  if (partition)
  {
    // 示例：擦除分区以准备写入
    return esp_partition_erase_range(partition, 0, partition->size) == ESP_OK;
  }
  return false;
}
bool SQUpdateClass::_verifyEnd()
{
  // 示例：检查更新是否完成
  return _progress == _size;
}
bool SQUpdateClass::_verifyHeader(uint8_t data)
{
  // 示例：检查头部数据是否包含预期的magic byte
  return data == 0xE9; // 替换为您实际的magic byte值
}

SQUpdateClass sqUpdate;