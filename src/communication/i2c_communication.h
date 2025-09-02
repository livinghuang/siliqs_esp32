#pragma once
#include "Arduino.h"
#include "bsp.h"
#define DEFAULT_I2C_FREQUENCY 400000
#define I2C_MAX_DATA_LENGTH 255

#ifndef pI2C_SDA
#define pI2C_SDA 18
#endif
#ifndef pI2C_SCL
#define pI2C_SCL 19
#endif

typedef struct
{
  uint8_t address;                   // I2C 设备地址
  uint8_t data[I2C_MAX_DATA_LENGTH]; // I2C 数据
  uint16_t length;                   // 数据长度
} i2c_data_t;

class I2CCommunication
{
public:
  // Constructor
  I2CCommunication(int sda = pI2C_SDA, int scl = pI2C_SCL, int frequency = DEFAULT_I2C_FREQUENCY);

  // Send I2C data frame
  void send_i2c(const i2c_data_t *i2cData);

  // Receive I2C data frame
  size_t receive_i2c(i2c_data_t *i2cData, size_t length = I2C_MAX_DATA_LENGTH, int timeout = -1);

  // Print I2C data
  void print_data(i2c_data_t *i2cData);

  // Scan I2C bus for devices
  void scan_bus();
};
