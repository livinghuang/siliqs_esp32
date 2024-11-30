#pragma once

#include "bsp.h"
#include "sensor_measurement.h"

#ifdef USE_PTC
#include "siliqs_esp32.h"

#ifdef USE_MAX31865
#include "sensors/max31865/Adafruit_MAX31865.h"

/**
 * @class PTCMeasurement
 * @brief A class for managing PTC (Platinum Temperature Coefficient) sensor measurements using MAX31865.
 */
class PTCMeasurement : public Sensor
{
public:
  /**
   * @brief Constructor to initialize the PTCMeasurement object.
   * @param nominalRes The nominal resistance of the RTD (default: 100.0 ohms for PT100).
   * @param refRes The reference resistance used for MAX31865 circuit (default: 430.0 ohms).
   * @param numWires RTD wiring configuration (default: MAX31865_2WIRE).
   */
  PTCMeasurement(int spi_cs, int spi_mosi, int spi_miso, int spi_clk, float nominalRes = 100.0, float refRes = 400.0, max31865_numwires_t numWires = MAX31865_3WIRE);

  /**
   * @brief Initializes the PTC sensor and its associated hardware.
   */
  void begin() override;

  /**
   * @brief Fetches the latest temperature measurement from the PTC sensor.
   */
  void getMeasurement() override;

  /**
   * @brief Get the most recent temperature measurement.
   * @return The last measured temperature in degrees Celsius.
   */
  float getTemperature() const { return temperature; }

private:
  Adafruit_MAX31865 max31865;   // Instance of the MAX31865 sensor class
  float temperature = -273.15;  // Last recorded temperature (initialized to absolute zero)
  float nominalResistance;      // Nominal resistance of the RTD
  float referenceResistance;    // Reference resistance for the MAX31865 circuit
  max31865_numwires_t numWires; // RTD wiring configuration
};

#endif // USE_MAX31865
#endif // USE_PTC
