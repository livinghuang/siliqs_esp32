#pragma once
#define USE_MAGNETOMETER
#define USE_2_WIRE_SENSOR_BUS
#define MQC5883_I2C_ADDRESS 0x0D // 假设使用 MQC5883 传感器的 I2C 地址
#define pMQC5883_I2C_SCL 19
#define pMQC5883_I2C_SDA 18