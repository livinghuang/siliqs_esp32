#include "bsp.h"
#ifdef USE_RS485
#include "siliqs_esp32.h"
#include "rs485_communication.h"

RS485Communication::RS485Communication(HardwareSerial &serial, int baudRate, int RO, int DI, int directionPin, int powerPin, bool highActive)
    : _serial(serial), _baudRate(baudRate), _RO(RO), _DI(DI), _directionPin(directionPin), _powerPin(powerPin), _powerPinHighActive(highActive)
{
  _serial.setTimeout(1000); // 默认超时1秒
  console.log(sqDEBUG, "RS485 Start baud rate: %d", _baudRate);
}

void RS485Communication::begin()
{
  powerOn(); // Power on if powerPin is set
  pinMode(_RO, INPUT);
  pinMode(_DI, OUTPUT);
  pinMode(_directionPin, OUTPUT);

  _serial.begin(_baudRate, SERIAL_8N1, _RO, _DI); // Use _serial
}

void RS485Communication::send(const char *data, int length)
{
  powerOn();        // Power on if powerPin is set
  enableTransmit(); // Enable transmission
  delay(1);
  _serial.write(data, length); // Use _serial
  _serial.flush();
}

int get_reasonable_timeout(int baudRate, int length)
{
  int timeout = (int)((length * 8 + 10) * (1000.0 / baudRate)) * 2;
  if (timeout < 1000)
  {
    timeout = 1000;
  }
  else if (timeout > 10000)
  {
    timeout = 10000;
  }
  return timeout;
}

size_t RS485Communication::readFromChar(char *buffer, char start_char, size_t length, int timeout)
{
  console.log(sqDEBUG, "RS485 readFromChar start_char: %x", start_char);
  console.log(sqDEBUG, "RS485 readFromChar target length: %d", length);
  // If timeout is -1, calculate a reasonable timeout based on baud rate and length
  timeout = (timeout == -1) ? get_reasonable_timeout(_baudRate, length) : timeout;
  enableReceive(); // Switch to receive mode

  // Clear buffer before receiving new data
  memset(buffer, 0, length);

  // Read data from _serial
  size_t lengthReceived = 0;

  uint32_t start = millis();
  while (1)
  {
    if (_serial.available())
    {
      buffer[lengthReceived] = _serial.read();
      if (buffer[0] == start_char)
      {
        lengthReceived++;
      }
    }
    if (lengthReceived == length)
    {
      break;
    }
    if (millis() - start > timeout)
    {
      console.log(sqDEBUG, "Timeout occurred! timeout: %d, lengthReceived: %d", timeout, lengthReceived);
      break;
    }
  }

  if (lengthReceived > 0)
  {
    console.log(sqDEBUG, "RS485 Received: ");
    console.log(sqDEBUG, (uint8_t *)buffer, lengthReceived); // Print received data in bytes
    return lengthReceived;
  }
  else
  {
    console.log(sqDEBUG, "no data received from rs485");
    return 0;
  }
}

size_t RS485Communication::receive(char *buffer, size_t length, int timeout)
{
  // If timeout is -1, calculate a reasonable timeout based on baud rate and length
  timeout = (timeout == -1) ? get_reasonable_timeout(_baudRate, length) : timeout;
  enableReceive();             // Switch to receive mode
  _serial.setTimeout(timeout); // Set a timeout

  // Clear buffer before receiving new data
  memset(buffer, 0, length);

  // Read data from _serial
  size_t lengthReceived = _serial.readBytes((uint8_t *)buffer, length);
  if (lengthReceived > 0)
  {
    console.log(sqDEBUG, "RS485 Received: ");
    console.log(sqDEBUG, (uint8_t *)buffer, lengthReceived); // Print received data in bytes
    return lengthReceived;
  }
  else
  {
    console.log(sqDEBUG, "no data received from rs485");
    return 0;
  }
}
void RS485Communication::print(const String &data)
{
  send(data.c_str(), data.length());
}

void RS485Communication::println(const String &data)
{
  send(data.c_str(), data.length());
  send("\n", 1);
}

// 接收 RS485 String 数据，直到指定结束字符
String RS485Communication::readStringUntil(char end)
{
  char buf[RS485_MAX_DATA_LENGTH];
  int length = receive(buf, sizeof(buf));
  if (length > 0)
  {
    return String(buf).substring(0, length - 1);
  }
  else
  {
    return String();
  }
}

void RS485Communication::setReceiveTimeout(int timeout)
{
  _serial.setTimeout(timeout); // 设置串口接收超时时间
}

void RS485Communication::enableTransmit()
{
  pinMode(_directionPin, OUTPUT);
  digitalWrite(_directionPin, HIGH); // Switch to transmit mode using _directionPin
}

void RS485Communication::enableReceive()
{
  pinMode(_directionPin, OUTPUT);
  digitalWrite(_directionPin, LOW); // Switch to receive mode using _directionPin
}

void RS485Communication::powerOn()
{
  if (_powerPin != -1)
  {
    pinMode(_powerPin, OUTPUT);
    if (_powerPinHighActive)
    {
      digitalWrite(_powerPin, HIGH); // 打开电源
    }
    else
    {
      digitalWrite(_powerPin, LOW); // 打开电源
    }
  }
}
void RS485Communication::powerOff()
{
  if (_powerPin != -1)
  {
    if (_powerPinHighActive)
    {
      digitalWrite(_powerPin, LOW); // 關閉电源
    }
    else
    {
      digitalWrite(_powerPin, HIGH); // 關閉电源
    }
  }
}
RS485Communication::~RS485Communication()
{
  powerOff(); // Power off when the object is destroyed
}
#endif // USE_RS485