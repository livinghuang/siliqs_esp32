#pragma once
#define USE_TEMPERATURE
#define USE_HDC1080_I2C
#define USE_2_WIRE_SENSOR_BUS
#define HDC1080_I2C_ADDRESS 0x40 // 假设使用 HDC1080 传感器的 I2C 地址
#define pHDC1080_I2C_SCL 19
#define pHDC1080_I2C_SDA 18
#define pVext 1