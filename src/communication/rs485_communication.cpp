#include "bsp.h"
#ifdef USE_RS485
#include "rs485_communication.h"
void print_bytes(const uint8_t *data, int length)
{
  for (int i = 0; i < length; ++i)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
RS485Communication::RS485Communication(HardwareSerial &serial, int baudRate, int RO, int DI, int directionPin, int powerPin)
    : _serial(serial), _baudRate(baudRate), _RO(RO), _DI(DI), _directionPin(directionPin)
{
  this->powerPin = powerPin;
}

void RS485Communication::begin()
{
  if (powerPin != -1)
  {
    powerOn(); // Power on if powerPin is set
  }
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, HIGH); // Power on

  pinMode(_RO, INPUT);
  pinMode(_DI, OUTPUT);
  pinMode(_directionPin, OUTPUT);

  _serial.begin(_baudRate, SERIAL_8N1, _RO, _DI); // Use _serial
}

void RS485Communication::send(const char *data)
{
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, HIGH); // Power on

  Serial.println("send");
  enableTransmit(); // Enable transmission
  delay(1);
  _serial.write(data, strlen(data)); // Use _serial
  _serial.flush();
}

void RS485Communication::receive(char *buffer, int maxLength, int timeout)
{
  enableReceive();             // Switch to receive mode
  _serial.setTimeout(timeout); // Set a timeout

  // Clear buffer before receiving new data
  memset(buffer, 0, maxLength);

  // Read up to 'maxLength-1' bytes or until newline is encountered
  int bytesRead = _serial.readBytesUntil('\n', buffer, maxLength - 1);

  if (bytesRead > 0)
  {
    Serial.print("Received: ");
    print_bytes((uint8_t *)buffer, bytesRead); // Print received data in bytes
  }
  else
  {
    Serial.println("No data received or timeout occurred.");
  }
}

void RS485Communication::send_modbus(modbus_t *mb)
{
  // 打印 Modbus 帧以供调试
  print_modbus_frame(mb);

  enableTransmit(); // 启用传输模式

  // 发送 Modbus 帧
  _serial.write(mb->address);  // 发送地址
  _serial.write(mb->function); // 发送功能码

  // 发送数据数组中的每个字节
  for (uint8_t i = 0; i < mb->length; i++)
  {
    _serial.write(mb->data[i]);
  }

  // 计算并追加 CRC
  crc(mb);                              // 计算正确的 CRC
  _serial.write((mb->crc >> 8) & 0xFF); // 发送 CRC 高字节
  _serial.write(mb->crc & 0xFF);        // 发送 CRC 低字节

  _serial.flush(); // 确保所有数据都已发送
  enableReceive(); // 切换回接收模式
}

void RS485Communication::receive_modbus(modbus_t *mb, int timeout)
{
  enableReceive();             // Enable receive mode
  _serial.setTimeout(timeout); // Set timeout

  // Read Modbus frame
  mb->address = _serial.read();
  mb->function = _serial.read();

  for (uint8_t i = 0; i < mb->length; i++)
  {
    uint8_t highByte = _serial.read();
    uint8_t lowByte = _serial.read();
    mb->data[i] = (highByte << 8) | lowByte;
  }

  // Read and verify CRC
  uint16_t receivedCrc = (_serial.read() << 8) | _serial.read();
  crc(mb); // Recalculate CRC

  if (mb->crc != receivedCrc)
  {
    Serial.println("CRC mismatch!");
    // Handle CRC error (return or set error flag)
  }
  else
  {
    Serial.println("Modbus message received successfully.");
  }
}

void RS485Communication::print_modbus_frame(modbus_t *mb)
{
  Serial.print("Address: ");
  Serial.println(mb->address, HEX); // Print address in hexadecimal

  Serial.print("Function: ");
  Serial.println(mb->function, HEX); // Print function code in hexadecimal

  Serial.println("Data: ");
  for (uint8_t i = 0; i < mb->length; i++)
  {
    Serial.print(mb->data[i], HEX); // Print each byte in data array as hexadecimal
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("CRC: ");
  Serial.println(mb->crc, HEX); // Print CRC in hexadecimal
}

void RS485Communication::crc(modbus_t *mb)
{
  uint16_t crc = 0xFFFF; // 初始化 CRC 为 0xFFFF

  // 处理地址和功能码
  crc ^= mb->address; // 将地址与 CRC 异或
  update_crc(&crc);   // 更新 CRC

  crc ^= mb->function; // 将功能码与 CRC 异或
  update_crc(&crc);    // 更新 CRC

  // 处理数据字节
  for (uint8_t i = 0; i < mb->length; i++)
  {
    crc ^= mb->data[i]; // 将每个数据字节与 CRC 异或
    update_crc(&crc);   // 更新 CRC
  }

  // 将计算得到的 CRC 存储到 Modbus 帧中
  mb->crc = crc;
}

// CRC 更新的辅助函数，根据 Modbus 算法
void RS485Communication::update_crc(uint16_t *crc)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (*crc & 0x0001)
    {                 // 如果最低位为 1
      *crc >>= 1;     // 右移一位
      *crc ^= 0xA001; // 异或 Modbus 多项式
    }
    else
    {
      *crc >>= 1; // 如果最低位不是 1，只右移一位
    }
  }
}

void RS485Communication::enableTransmit()
{
  digitalWrite(_directionPin, HIGH); // Switch to transmit mode using _directionPin
}

void RS485Communication::enableReceive()
{
  digitalWrite(_directionPin, LOW); // Switch to receive mode using _directionPin
}

void RS485Communication::powerOn()
{
  if (powerPin != -1)
  {
    Serial.println("powerOn");
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH); // Power on
  }
}

void RS485Communication::powerOff()
{
  if (powerPin != -1)
  {
    digitalWrite(powerPin, LOW); // Power off
  }
}

RS485Communication::~RS485Communication()
{
  powerOff(); // Power off when the object is destroyed
}
#endif // USE_RS485