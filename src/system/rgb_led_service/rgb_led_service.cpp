#include "bsp.h"
#ifdef USE_RGB_LED
#include "rgb_led_service.h"

const int color_off[] = {0x00, 0x00, 0x00};
const int color_red[] = {0x00, 0xFF, 0x00};
const int color_green[] = {0xFF, 0x00, 0x00};
const int color_blue[] = {0x00, 0x00, 0xFF};
const int color_white[] = {0x77, 0x77, 0x77};

RGBLedService::RGBLedService(int _pin, int _numLeds)
    : pin(_pin), numLeds(_numLeds), led_data(nullptr)
{
  // Initialize RMT with given pin and settings
  if (!rmtInit(pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000))
  {
    console.log(sqINFO, "RMT initialization failed!");
  }
  else
  {
    console.log(sqINFO, "RMT initialized successfully.");

    // Allocate memory for LED data (24 bits per LED)
    led_data = new rmt_data_t[numLeds * 24];
    if (!led_data)
    {
      console.log(sqERROR, "Failed to allocate memory for LED data.");
    }
  }
}

RGBLedService::~RGBLedService()
{
  if (led_data)
  {
    delete[] led_data;
    led_data = nullptr;
  }

  rmtDeinit(pin); // Cleanup RMT if required
}

void RGBLedService::begin()
{
  // Perform any library-specific initialization
  console.log(sqINFO, "RGBLedService initialized.");
}
void RGBLedService::set_led_bit(rmt_data_t *led_data, bool bit_value)
{
  if (bit_value)
  {
    led_data->level0 = 1;    // High level for "1"
    led_data->duration0 = 8; // Duration of high level for "1"
    led_data->level1 = 0;    // Low level for "1"
    led_data->duration1 = 4; // Duration of low level for "1"
  }
  else
  {
    led_data->level0 = 1;    // High level for "0"
    led_data->duration0 = 4; // Duration of high level for "0"
    led_data->level1 = 0;    // Low level for "0"
    led_data->duration1 = 8; // Duration of low level for "0"
  }
}
void RGBLedService::set_led(const int color[])
{
  // color[] is a 3-element array containing RGB values
  // Example color[] = {0xFF, 0x00, 0x00}; // Red

  // create the color data
  int i = 0;
  for (int led = 0; led < numLeds; led++)
  {
    for (int col = 0; col < 3; col++)
    {
      for (int bit = 0; bit < 8; bit++)
      {
        bool bit_value = (color[col] & (1 << (7 - bit))) &&
                         (led == led_index);
        set_led_bit(&led_data[i], bit_value);
        i++;
      }
    }
  }
  // Send data via RMT
  rmtWrite(pRGB_LED, led_data, numLeds * 24, RMT_WAIT_FOR_EVER);
  vTaskDelay(100);
}
#endif