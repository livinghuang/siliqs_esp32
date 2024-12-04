#pragma once
#define USE_TEMPERATURE
#ifdef USE_TEMPERATURE
#define USE_HDC1080_I2C
#define USE_2_WIRE_SENSOR_BUS
#define HDC1080_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址
#define pHDC1080_I2C_SCL 19
#define pHDC1080_I2C_SDA 18
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
