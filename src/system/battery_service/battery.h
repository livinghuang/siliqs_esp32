#pragma once
#include "bsp.h"
#ifdef USE_BATTERY_SERVICE
#include <Arduino.h>
#include "utility.h"

struct battery_adc_correction_struct
{
  float factor; // 用於電池電壓的 ADC 校正因子
  float offset; // 用於電池電壓的 ADC 校正偏移量
};

#define POWER_NORMAL 0
#define POWER_SAVING 1
#define POWER_CHARGING 2

struct battery_struct // 2 bytes
{
  uint8_t mode;  // 0: normal , 1: power saving , 2: charging
  uint8_t level; // 0-255
};

class battery
{
public:
  void set_battery_mode(uint8_t mode)
  {
    data.mode = mode;
  }
  void set_power_mode_charging(float voltage)
  {
    power_mode_charging = voltage;
  }

private:
  bool print_log = false;
  float power_mode_min = 3.35;
  float power_mode_full = 4.1;
  float power_mode_charging = 4.5;
  float power_mode_normal = 3.7;
  float vref = 3.3; // ADC 參考電壓
  int battery_percentage;
  float battery_voltage;
  uint8_t battery_level;
  int gpio;
  int fetch_times;
  uint32_t update_interval_ms = 1000;
  TaskHandle_t taskHandle = nullptr;
  float adc_correction_factor = 1.0;
  float adc_correction_offset = 0.0;

  struct battery_struct data;
  float read(int gpio)
  {
    pinMode(gpio, ANALOG);
    int raw = analogRead(gpio);
    float voltage;
    if (!use_adc_correction)
    {
      voltage = ((float)raw * vref / 2048.0);
    }
    else
    {
      voltage = ((float)raw * vref / 2048.0) * adc_correction_factor + adc_correction_offset;
    }
    if (print_log)
    {
      log("ADC Raw: " + String(raw));
      log(String(voltage));
    }
    return voltage;
  }

  uint8_t calc_battery_level(float voltage)
  {
    if (voltage >= power_mode_full)
      return 255;
    if (voltage <= power_mode_min)
      return 0;

    float ratio = (voltage - power_mode_min) / (power_mode_full - power_mode_min);
    return (uint8_t)(ratio * 255.0);
  }

  void set_power_mode(float battery_voltage)
  {
    if (battery_voltage < power_mode_min)
    {
      data.mode = POWER_SAVING;
    }
    else if (battery_voltage > power_mode_charging)
    {
      data.mode = POWER_CHARGING;
    }
    else
    {
      data.mode = POWER_NORMAL;
    }
  }

  void set_battery_level(uint8_t level)
  {
    data.level = level;
  }

  static void taskLoop(void *param)
  {
    battery *instance = static_cast<battery *>(param);
    while (true)
    {
      instance->fetch_adc();
      if (instance->print_log)
      {
        instance->print();
      }
      vTaskDelay(instance->update_interval_ms / portTICK_PERIOD_MS); // 根據使用者設定更新週期
    }
  }

public:
  HardwareSerial *serial = nullptr;
  bool use_adc_correction = true;

  battery(bool _print_log = false, HardwareSerial &serial = Serial)
  {
    this->print_log = _print_log;
    this->serial = &serial;

    if (_print_log)
    {
      if (serial)
      {
        Serial.println("You will see \"log start success\" in another serial port");
        log("log start success");
      }
      else
      {
        Serial.println("log failure");
      }
    }
  }

  void log(const String data)
  {
    log(data.c_str());
  }
  void log(const char *data)
  {
    if (print_log)
    {
      serial->println(String("[bat] ") + data);
      serial->flush();
    }
  }

  void begin(int pin, int times = 5)
  {
    gpio = pin;
    fetch_times = times;
    battery_voltage = 0.0;
    battery_level = 0;
    data.mode = POWER_NORMAL;
    data.level = 0;
    xTaskCreatePinnedToCore(
        taskLoop,
        "battery_task",
        2048,
        this,
        1,
        &taskHandle,
        0);
  }

  void end()
  {
    if (taskHandle != nullptr)
    {
      vTaskDelete(taskHandle);
      taskHandle = nullptr;
    }
  }

  void fetch_adc()
  {
    float voltage = 0.0;
    float adc = 0.0;
    log("GPIO PIN:" + String(gpio));

    for (int i = 0; i < fetch_times; i++)
    {
      adc = read(gpio);
      voltage += adc;
      log(String(adc));
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    battery_voltage = voltage / fetch_times;
    log("ADC average: " + String(battery_voltage));
    set_power_mode(battery_voltage);
    battery_level = calc_battery_level(battery_voltage);
    set_battery_level(battery_level);
  }

  void get_battery_data(struct battery_struct *_data)
  {
    memcpy(_data, &data, sizeof(data));
  }
  uint8_t get_battery_level()
  {
    return data.level;
  }
  uint8_t get_battery_mode()
  {
    return data.mode;
  }
  float get_battery_voltage()
  {
    return battery_voltage;
  }
  void print(void)
  {
    Serial.println("------battery status------");
    switch (data.mode)
    {
    case POWER_SAVING:
      Serial.println("POWER_SAVING");
      break;
    case POWER_NORMAL:
      Serial.println("NORMAL");
      break;
    case POWER_CHARGING:
      Serial.println("CHARGING");
      break;
    }
    Serial.println("Battery data level: " + String(data.level));
    Serial.println("Battery voltage: " + String(battery_voltage));
    Serial.print("RAW: ");
    print_hex((uint8_t *)&data, sizeof(data));
    Serial.println("------battery status------");
  }

  void calibrate(float real_voltage)
  {
    float raw_voltage = read(gpio);
    if (raw_voltage > 0)
    {
      adc_correction_factor = real_voltage / raw_voltage;
      log("Calibration complete: factor = " + String(adc_correction_factor));
    }
  }

  void set_correction(float factor, float offset)
  {
    adc_correction_factor = factor;
    adc_correction_offset = offset;
    log("Manual correction set: factor = " + String(factor) + ", offset = " + String(offset));
  }

  void set_power_mode_min(float voltage)
  {
    power_mode_min = voltage;
  }

  void set_power_mode_full(float voltage)
  {
    power_mode_full = voltage;
  }

  void enable_log(bool enable)
  {
    print_log = enable;
  }

  void set_update_interval(uint32_t interval_ms)
  {
    update_interval_ms = interval_ms;
  }

  void set_adc_reference(float ref_voltage)
  {
    vref = ref_voltage;
  }

  /*
  Enter calibration mode in factory settings.
  This function is for calibration the battery voltage in factory mode.
  It will define the battery slope and offset.
  */
  void enter_calibration_in_factory_mode(float real_highest_voltage, float real_lowest_voltage)
  {
    // under development
  }

  ~battery()
  {
    end();
  }
};

extern battery batt;
void test_battery(void);

#endif