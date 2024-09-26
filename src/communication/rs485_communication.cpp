#ifdef USE_RS485
#include "rs485_communication.h"

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
}

void RS485Communication::receive(char *buffer, int length)
{
  enableReceive(); // Switch to receive mode
  int index = 0;
  while (_serial.available() > 0 && index < length - 1)
  {
    buffer[index++] = _serial.read();
  }
  buffer[index] = '\0'; // Null-terminate the buffer
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