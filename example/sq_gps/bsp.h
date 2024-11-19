#ifndef BSP_H
#define BSP_H
#define USE_GPS
#ifdef USE_GPS
#define GPS_TX 1
#define GPS_RX 0
#define GPS_BAUD 9600
#define GPS_VEXT 1
#endif
// if you want to use custom pins, please define CUSTOM_PINS in here, otherwise, it will use pins defined in pins_defined.h
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

#endif