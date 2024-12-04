#ifndef BSP_H
#define BSP_H
#define USE_LORAWAN
#define LORA_DIO1 3
#define LORA_BUSY 4
#define LORA_NRST 5
#define LORA_MISO 6
#define LORA_MOSI 7
#define LORA_NSS 8
#define LORA_SCK 10
#define REGION (LoRaWANBand_t) AS923
#define SUB_BAND 0

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