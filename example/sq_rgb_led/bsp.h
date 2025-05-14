#pragma once
#define USE_RGB_LED
#define pRGB_LED 17
/*
This LED library is derived from the esp32-hal-rgb-led library. It supports driving WS2812B LED strips and can be modified to control other types of LED strips as needed.

The implementation files are located at:

src/system/rgb_led_service/rgb_led_service.h
src/system/rgb_led_service/rgb_led_service.cpp

This project is workable and it has been tested. But it may not suitable place at "src/system" folder. we may move to peripherals folder in future.
*/