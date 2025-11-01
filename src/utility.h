#pragma once
#include "Arduino.h"

class Alarm
{
public:
  Alarm() = default;

  void init(uint8_t pin, bool low_active = false)
  {
    if (pin >= NUM_DIGITAL_PINS)
    {
      Serial.println("Error: Invalid pin number for Alarm");
      return;
    }
    pAlarm = pin;
    this->low_active = low_active;
    pinMode(pAlarm, OUTPUT);
    off(); // 預設為關閉狀態
  }

  void on()
  {
    if (!isInitialized())
      return;
    digitalWrite(pAlarm, low_active ? LOW : HIGH);
  }

  void off()
  {
    if (pAlarm == 255)
    {
      Serial.println("Error: Alarm pin not initialized");
      return;
    }
    digitalWrite(pAlarm, low_active ? HIGH : LOW);
  }

  void toggle()
  {
    if (!isInitialized())
      return;
    digitalWrite(pAlarm, !digitalRead(pAlarm));
  }

  void end()
  {
    if (!isInitialized())
      return;
    off();
    pinMode(pAlarm, INPUT);
  }

private:
  bool low_active = false;
  uint8_t pAlarm = 255; // 無效值代表尚未初始化
  bool isInitialized()
  {
    if (pAlarm == 255)
    {
      Serial.println("Error: Alarm pin not initialized");
      return false;
    }
    return true;
  }
};

bool is_Printable(uint8_t c);
void print_data(uint8_t data);
void print_data(const void *data, size_t len);
void print_hex(uint8_t data);
void print_hex(const void *data, size_t len);
String to_hex_string(const void *data, size_t len);

void swap_high_low_bytes(uint16_t *a);
uint16_t swap_high_low_bytes(uint16_t a);
void print_reverse_endian(uint16_t a);
uint16_t calculateCRC(const uint8_t *data, size_t length);
void calculateCRC(const uint8_t *data, size_t length, uint16_t *result_crc);

void vector_to_buffer();
void buffer_to_vector();
void put_data_to_vector(uint8_t *data, size_t len);
size_t get_data_from_vector(uint8_t *data, size_t len);
bool is_vector_empty();
size_t get_vector_size();
void print_top_vector_data();
