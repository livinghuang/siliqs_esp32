#pragma once
#include "bsp.h"
#ifdef USE_LED
#include <Arduino.h>

class led
{
public:
  void begin(uint8_t pin, bool activeLow = true, uint32_t intervalMs = 200)
  {
    gpioPin = pin;
    isActiveLow = activeLow;
    blinkInterval = intervalMs;

    pinMode(gpioPin, OUTPUT);
    off();

    xTaskCreatePinnedToCore(
        taskLoop,
        "led_task",
        512,
        this,
        1,
        &taskHandle,
        0);
  }

  void blink(int times = 3)
  {
    blinkCount = times;
    blinking = true;
  }

  void on()
  {
    blinking = false;
    digitalWrite(gpioPin, isActiveLow ? LOW : HIGH);
  }

  void off()
  {
    blinking = false;
    digitalWrite(gpioPin, isActiveLow ? HIGH : LOW);
  }

  void end()
  {
    if (taskHandle != nullptr)
    {
      vTaskDelete(taskHandle);
      taskHandle = nullptr;
      off(); // 關掉 LED
    }
  }

private:
  uint8_t gpioPin;
  bool isActiveLow;
  uint32_t blinkInterval;
  TaskHandle_t taskHandle = nullptr;
  volatile bool blinking = false;
  volatile int blinkCount = 0;

  static void taskLoop(void *param)
  {
    led *instance = static_cast<led *>(param);
    while (true)
    {
      if (instance->blinking && instance->blinkCount > 0)
      {
        instance->toggle();
        instance->blinkCount = instance->blinkCount - 1;
        vTaskDelay(instance->blinkInterval / portTICK_PERIOD_MS);
        instance->toggle();
        vTaskDelay(instance->blinkInterval / portTICK_PERIOD_MS);
      }
      else
      {
        vTaskDelay(10 / portTICK_PERIOD_MS); // idle delay
      }
    }
  }

  void toggle()
  {
    int current = digitalRead(gpioPin);
    digitalWrite(gpioPin, !current);
  }
};

extern class led led;
void test_led();
#endif