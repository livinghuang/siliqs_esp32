#pragma once
#define USE_BATTERY
#ifdef USE_BATTERY
#define USE_BAT_ADC
#define BAT_ADC_PIN 2
#define BAT_MAX_VOLTAGE 4.2
#define BAT_MIN_VOLTAGE 3.0
#define BAT_VOLTAGE_MULTIPLIER 1.0 // if the value is 1.0, the battery voltage will be multiplied by 1.0, otherwise, the battery voltage will be multiplied by the value
#endif
/*
below is the example:
// #define CUSTOM_PINS
// #define pVext 1
// #define pADC_BAT 2
// #define pSDA 18
// #define pSCL 19
// #define pLORA_RST 5
// #define pLED 0

// #define pCS 3
// #define pMOSI 7
// #define pMISO 6
// #define pSCK 10

// // RS485
// #define pRS485_RO 19 // Receiver Output (RO) connected to pin 19
// #define pRS485_DI 18 // Driver Input (DI) connected to pin 18
// #define pRS485_DE 4  // Data Enable (DE) connected to pin 4

// // 24 POWER
// #define pSDN 3
*/
