#include "bsp.h"
#pragma once
#ifdef USE_RGB_LED // Only compile when USE_RGB_LED is enabled
#ifndef RGB_LED_SERVICE_H
#define RGB_LED_SERVICE_H

#include "siliqs_heltec_esp32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32-hal-rgb-led.h"
// Number of LEDs and bits per LED
#define NR_OF_LEDS 1
#define NR_OF_ALL_BITS (24 * NR_OF_LEDS)

// Color enumeration for better readability
enum COLOR
{
  OFF = 0,
  RED,
  GREEN,
  BLUE,
  WHITE
};

class RGBLedService
{
public:
  RGBLedService(int pin, int numLeds);
  ~RGBLedService();
  // Predefined RGB values for different colors

  rmt_data_t *led_data;
  int led_index;
  void begin(); // Initialize the LED service
  void set_led_bit(rmt_data_t *led_data, bool bit_value);
  void set_led(const int color[]);

private:
  int pin;
  int numLeds;
};

#endif
#endif