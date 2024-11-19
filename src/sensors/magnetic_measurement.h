#pragma once
#include "bsp.h"

#ifdef USE_MAGNETOMETER
#include "siliqs_heltec_esp32.h"
#include "sensor_measurement.h"
#include <Wire.h>
#include "sensors/mqc5883/MechaQMC5883.h"

// Magnetometer class derived from Sensor base class
class Magnetometer : public Sensor
{
private:
  const uint8_t addr; // I2C address of the magnetometer
  const int sdaPin;   // SDA pin for I2C
  const int sclPin;   // SCL pin for I2C
  MechaQMC5883 mag;   // Instance of the MechaQMC5883 driver

public:
  /**
   * @brief Constructor for the Magnetometer class
   * @param address I2C address of the QMC5883
   * @param sda SDA pin
   * @param scl SCL pin
   */
  Magnetometer(uint8_t address = QMC5883_ADDR, int sda = pMQC5883_I2C_SDA, int scl = pMQC5883_I2C_SCL);

  /**
   * @brief Initialize the magnetometer
   */
  void begin() override;

  /**
   * @brief Get the magnetic field measurements
   */
  void getMeasurement() override;

  ~Magnetometer() override {} // Virtual destructor

  int16_t x = 0, y = 0, z = 0; // Public access to store measurements
};

#endif
