#pragma once
#define USE_LORA
/*
We use a forked version of the RadioLib library (original repository: https://github.com/jgromes/RadioLib) to implement the LoRaWAN service.
Our fork includes modifications to support the AS923_1 band, which is not available in the original library.
When operating in the AS923_1 band, the transmission frequencies are as follows:
923.200 MHz,923.400 MHz,923.600 MHz,923.800 MHz,924.000 MHz,924.200 MHz,924.400 MHz,924.600 MHz
The source code for these modifications can be found in the file src/radiolab/src/protocols/LoRaWAN/LoRaWANBands.cpp.
*/
#define LORA_DIO1 3
#define LORA_BUSY 4
#define LORA_NRST 5
#define LORA_MISO 6
#define LORA_MOSI 7
#define LORA_NSS 8
#define LORA_SCK 10

//  #define LORA_DIO1  14
//  #define LORA_BUSY  13
//  #define LORA_NRST  12
//  #define LORA_MISO  11
//  #define LORA_MOSI  10
//  #define LORA_SCK =9
//  #define LORA_NSS =8
