#include "i2c_communication.h"
#include <Arduino.h>
#include <Wire.h>

// 推斷可用的 Wire 內部緩衝大小，做寫入/讀取分段
#ifndef I2C_WIRE_CHUNK_MAX
#if defined(I2C_BUFFER_LENGTH)
#define I2C_WIRE_CHUNK_MAX I2C_BUFFER_LENGTH
#elif defined(WIRE_BUFFER_LENGTH)
#define I2C_WIRE_CHUNK_MAX WIRE_BUFFER_LENGTH
#elif defined(BUFFER_LENGTH)
#define I2C_WIRE_CHUNK_MAX BUFFER_LENGTH
#else
#define I2C_WIRE_CHUNK_MAX 32
#endif
#endif

// 寫入時保守地留 1 byte 餘量，避免部分核心的 off-by-one
static inline size_t _tx_chunk_limit()
{
  return (I2C_WIRE_CHUNK_MAX > 16) ? (size_t)(I2C_WIRE_CHUNK_MAX - 1) : (size_t)16;
}

I2CCommunication::I2CCommunication(int sda, int scl, int frequency)
{
  // 初始化 I2C（依平台差異）
#if defined(ARDUINO_ARCH_ESP32)
  Wire.begin(sda, scl, (uint32_t)frequency);
#elif defined(ARDUINO_ARCH_ESP8266)
  Wire.begin((uint8_t)sda, (uint8_t)scl);
  Wire.setClock((uint32_t)frequency);
#else
  (void)sda;
  (void)scl;
  Wire.begin();
#if defined(WIRE_HAS_SET_CLOCK) || defined(TWBR)
  Wire.setClock((uint32_t)frequency);
#endif
#endif
}

// 傳送 I2C 資料（自動切塊避免超出 Wire 緩衝）
void I2CCommunication::send_i2c(const i2c_data_t *i2cData)
{
  if (!i2cData)
    return;
  uint8_t addr = i2cData->address;
  size_t len = i2cData->length;
  if (len > I2C_MAX_DATA_LENGTH)
    len = I2C_MAX_DATA_LENGTH;

  const uint8_t *p = i2cData->data;
  const size_t chunkMax = _tx_chunk_limit();

  size_t sent = 0;
  while (sent < len)
  {
    size_t chunk = (len - sent > chunkMax) ? chunkMax : (len - sent);
    Wire.beginTransmission(addr);
    size_t w = Wire.write(p + sent, chunk);
    // 對於超過 Buffer 的 case，write 可能回傳比 chunk 少
    if (w == 0)
    {
      // 無法寫入，結束傳輸並放棄
      Wire.endTransmission(true);
      break;
    }
    sent += w;
    // 若還有資料，先以 repeated-start（不送 STOP）結束本段
    // 最後一段才送 STOP
    uint8_t err = Wire.endTransmission((sent >= len) ? true : false);
    if (err != 0)
    {
      // 0=OK, 1=資料太長, 2=地址 NACK, 3=資料 NACK, 4=其他錯誤
      // 這裡不直接 Serial.print，以便在無串口時也能編譯。
      break;
    }
  }
}

// 接收 I2C 資料（單純從設備連續讀取 length bytes）
// 注意：若設備需要先寫入暫存器位址，請在呼叫本函式前自行以 send_i2c() 寫入索引。
size_t I2CCommunication::receive_i2c(i2c_data_t *i2cData, size_t length, int timeout)
{
  if (!i2cData)
    return 0;
  if (length > I2C_MAX_DATA_LENGTH)
    length = I2C_MAX_DATA_LENGTH;

#if defined(ARDUINO_ARCH_ESP32)
  if (timeout >= 0)
  {
    // ESP32 的 TwoWire 為 setTimeOut(ms)
    Wire.setTimeOut((uint16_t)timeout);
  }
#else
  (void)timeout; // 其他核心多半沒有 timeout API
#endif

  size_t totalRead = 0;
  const size_t chunkMax = I2C_WIRE_CHUNK_MAX;

  while (totalRead < length)
  {
    size_t chunk = (length - totalRead > chunkMax) ? chunkMax : (length - totalRead);
    // 大部分核心的 requestFrom 需要 uint8_t 長度
    size_t req = (chunk > 255) ? 255 : chunk;
#if ARDUINO >= 100
    // 第 3 參數為是否送 STOP；這裡每段都送 STOP，泛用性較高
    size_t got = Wire.requestFrom((int)i2cData->address, (int)req, (int)true);
#else
    size_t got = Wire.requestFrom(i2cData->address, (uint8_t)req);
#endif
    if (got == 0)
      break;

    // 逐一讀出
    for (size_t i = 0; i < got && totalRead < length && Wire.available(); ++i)
    {
      i2cData->data[totalRead++] = (uint8_t)Wire.read();
    }

    if (got < req)
    {
      // 沒拿到期望的字節數，提前結束
      break;
    }
  }

  i2cData->length = (uint16_t)totalRead;
  return totalRead;
}

// 以十六進位列印 i2c_data_t 內容（使用 Serial）
void I2CCommunication::print_data(i2c_data_t *i2cData)
{
  if (!i2cData)
    return;
  Serial.print(F("I2C addr=0x"));
  if (i2cData->address < 16)
    Serial.print('0');
  Serial.print(i2cData->address, HEX);
  Serial.print(F(", len="));
  Serial.print(i2cData->length);
  Serial.print(F(", data: "));
  for (uint16_t i = 0; i < i2cData->length; ++i)
  {
    uint8_t b = i2cData->data[i];
    if (b < 16)
      Serial.print('0');
    Serial.print(b, HEX);
    if (i + 1 < i2cData->length)
      Serial.print(' ');
  }
  Serial.println();
}

// 掃描 I2C 總線，列出回應的 7-bit 地址
void I2CCommunication::scan_bus()
{
  Serial.println(F("I2C scan start..."));
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; ++addr)
  {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission(true); // 送 STOP
    if (err == 0)
    {
      Serial.print(F("  Found device at 0x"));
      if (addr < 16)
        Serial.print('0');
      Serial.println(addr, HEX);
      ++found;
    }
    else if (err == 4)
    {
      Serial.print(F("  Unknown error at 0x"));
      if (addr < 16)
        Serial.print('0');
      Serial.println(addr, HEX);
    }
    delay(2); // 稍微放慢，避免影響邊界設備
  }
  Serial.print(F("I2C scan done. Devices found: "));
  Serial.println(found);
}