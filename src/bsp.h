#pragma once
#define USE_TEMPERATURE
#define USE_HUMIDITY
#define USE_HDC1080_I2C
#define USE_2_WIRE_SENSOR_BUS
#define HDC1080_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址
#define pHDC1080_I2C_SCL 19
#define pHDC1080_I2C_SDA 18
#define USE_AIR_PRESSURE
#define USE_DSP310_I2C
#define DSP310_I2C_ADDRESS 0x77 // 假设使用 DSP310 传感器的 I2C 地址
#define pDSP310_I2C_SCL 19
#define pDSP310_I2C_SDA 18
#define HDC1080_HEATER_ON_SEC 1
#define pVext 1 // 這是需要開關

#define USE_LORAWAN


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
#define REGION (LoRaWANBand_t) AS923_1
#define SUB_BAND 0