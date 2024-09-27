#include "bsp.h"
#ifdef USE_RS485
#include "rs485_communication.h"

void print_bytes(const uint8_t *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    // Print each byte in hexadecimal format with leading zeros
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // Print a newline character at the end
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

void RS485Communication::send(const char *data, int length)
{
  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, HIGH); // Power on

  Serial.println("rs485 send");
  print_bytes((uint8_t *)data, length);
  enableTransmit(); // Enable transmission
  delay(1);
  _serial.write(data, length); // Use _serial
  _serial.flush();
}

size_t RS485Communication::receive(char *buffer, size_t length, int timeout)
{
  enableReceive();             // Switch to receive mode
  _serial.setTimeout(timeout); // Set a timeout

  // Clear buffer before receiving new data
  memset(buffer, 0, length);

  // Read up to 'maxLength-1' bytes or until newline is encountered
  length = _serial.readBytesUntil('\n', buffer, length);

  if (length > 0)
  {
    Serial.print("rs485 Received: ");
    print_bytes((uint8_t *)buffer, length); // Print received data in bytes
    return length;
  }
  else
  {
    Serial.println("No data received or timeout occurred.");
    return 0;
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