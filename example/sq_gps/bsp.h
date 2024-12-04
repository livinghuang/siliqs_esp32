#ifndef BSP_H
#define BSP_H
#define USE_GPS
#ifdef USE_GPS
#define GPS_TX_PIN 21
#define GPS_RX_PIN 20
#define GPS_BAUD 9600
#define GPS_VCTRL_PIN 37
#endif
#endif