//
// Note: This example uses a board with 32 WS2812b LEDs chained one
//      after another, each RGB LED has its 24 bit value
//      for color configuration (8b for each color)
//
//      Bits encoded as pulses as follows:
//
//      "0":
//         +-------+              +--
//         |       |              |
//         |       |              |
//         |       |              |
//      ---|       |--------------|
//         +       +              +
//         | 0.4us |   0.85 0us   |
//
//      "1":
//         +-------------+       +--
//         |             |       |
//         |             |       |
//         |             |       |
//         |             |       |
//      ---+             +-------+
//         |    0.8us    | 0.4us |
#pragma once
#include "bsp.h"

#ifdef USE_RGB_LED // Only compile when USE_RGB_LED is enabled

#include "siliqs_esp32.h"
#include "esp32-hal-rgb-led.h"

// RMT timing constants for WS2812
#define T1H 8 // 0.8us high for "1"
#define T1L 4 // 0.4us low for "1"
#define T0H 4 // 0.4us high for "0"
#define T0L 8 // 0.8us low for "0"

// Default number of LEDs
#define DEFAULT_NUM_LEDS 1

class RGBLedService
{
public:
  void begin(int pin = pRGB_LED, int num_leds = DEFAULT_NUM_LEDS, const int *init_color = nullptr)
  {
    Serial.begin(115200);

    if (led_data != nullptr)
    {
      Serial.println("RGBLedService already initialized.");
      return;
    }

    rgb_pin = pin;
    numLeds = num_leds;
    led_data_size = numLeds * 24;
    led_data = new rmt_data_t[led_data_size];

    rmtDeinit(rgb_pin);
    delay(100);
    if (!rmtInit(rgb_pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000))
    {
      Serial.println("RMT init failed");
    }
    else
    {
      Serial.println("RMT initialized with 100ns tick");
    }

    const int default_color[3] = {0x00, 0x00, 0x00}; // Default OFF
    const int *color_to_use = init_color ? init_color : default_color;

    fillLedData(color_to_use, 0); // Light up first LED only
  }

  void fillLedData(const int color[3], int led_index)
  {
    int i = 0;
    for (int led = 0; led < numLeds; ++led)
    {
      for (int col = 0; col < 3; ++col) // GRB order for WS2812
      {
        for (int bit = 0; bit < 8; ++bit)
        {
          bool bit_on = (color[col] & (1 << (7 - bit))) && (led == led_index);
          led_data[i].level0 = 1;
          led_data[i].duration0 = bit_on ? T1H : T0H;
          led_data[i].level1 = 0;
          led_data[i].duration1 = bit_on ? T1L : T0L;
          ++i;
        }
      }
    }
  }

  void light()
  {
    rmtWrite(rgb_pin, led_data, led_data_size, RMT_WAIT_FOR_EVER);
    delay(100);
  }

  void end()
  {
    if (led_data)
    {
      const int default_color[3] = {0x00, 0x00, 0x00};
      fillLedData(default_color, 0);
      rmtWrite(rgb_pin, led_data, led_data_size, RMT_WAIT_FOR_EVER);
      delete[] led_data;
      led_data = nullptr;
    }
    Serial.println("RGBLedService stopped.");
    delay(100);
    rmtDeinit(rgb_pin);
  }

  ~RGBLedService()
  {
    delete[] led_data;
  }

private:
  int led_index = 0;
  int rgb_pin = pRGB_LED;
  int numLeds = DEFAULT_NUM_LEDS;
  int led_data_size = DEFAULT_NUM_LEDS * 24;
  rmt_data_t *led_data = nullptr;
};

#endif
