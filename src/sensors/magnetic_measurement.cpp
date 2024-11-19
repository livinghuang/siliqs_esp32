#include "bsp.h"
#ifdef USE_MAGNETOMETER
#include "magnetic_measurement.h"
#include <Arduino.h>
#include "sensors/magnetic_measurement.h"

/**
 * @brief Construct a new Magnetometer object
 *
 * @param address I2C address of the magnetometer
 * @param sda SDA pin for I2C
 * @param scl SCL pin for I2C
 */
Magnetometer::Magnetometer(uint8_t address, int sda, int scl)
    : addr(address), sdaPin(sda), sclPin(scl)
{
  mag.setAddress(address); // Set the address for the MechaQMC5883 driver
}

/**
 * @brief Initialize the magnetometer
 *
 * Configures I2C and sets the magnetometer to continuous measurement mode.
 */
void Magnetometer::begin()
{
  Wire.begin(sdaPin, sclPin);                               // Start I2C communication
  mag.init();                                               // Initialize QMC5883 sensor
  mag.setMode(Mode_Continuous, ODR_200Hz, RNG_8G, OSR_512); // Default mode settings
  Serial.println("Magnetometer initialized.");
}

/**
 * @brief Get magnetic field measurements
 *
 * Reads values from the magnetometer and stores them in `x`, `y`, and `z`.
 */
void Magnetometer::getMeasurement()
{
  // Cast x, y, z from int16_t* to int* to match the MechaQMC5883::read signature
  if (mag.read((int *)&x, (int *)&y, (int *)&z) == 0)
  {
    Serial.printf("Magnetometer Readings - X: %d, Y: %d, Z: %d\n", x, y, z);
  }
  else
  {
    Serial.println("Failed to read data from Magnetometer.");
  }
}
#endif