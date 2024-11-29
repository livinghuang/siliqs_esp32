#include "bsp.h"
#include "ptc_measurement.h"

#ifdef USE_PTC
#ifdef USE_MAX31865

/**
 * @brief Constructor for the PTCMeasurement class.
 * @param spi_cs Chip Select (CS) pin for SPI.
 * @param spi_mosi Master Out Slave In (MOSI) pin for SPI.
 * @param spi_miso Master In Slave Out (MISO) pin for SPI.
 * @param spi_clk Clock (SCK) pin for SPI.
 * @param nominalRes Nominal resistance of the RTD (default: 100.0 ohms for PT100).
 * @param refRes Reference resistance used for MAX31865 circuit (default: 430.0 ohms).
 * @param numWires RTD wiring configuration (default: MAX31865_3WIRE).
 */
PTCMeasurement::PTCMeasurement(int spi_cs, int spi_mosi, int spi_miso, int spi_clk, float nominalRes, float refRes, max31865_numwires_t numWires)
    : max31865(spi_cs, spi_mosi, spi_miso, spi_clk),
      nominalResistance(nominalRes),
      referenceResistance(refRes),
      numWires(numWires)
{
}

/**
 * @brief Initializes the PTC sensor and its associated hardware.
 */
void PTCMeasurement::begin()
{
  // Initialize the MAX31865 sensor with the specified wiring configuration
  if (!max31865.begin(numWires))
  {
    console.log(sqINFO, "Failed to initialize MAX31865!");
    return;
  }

  // Enable bias voltage and auto-conversion
  max31865.enableBias(true);
  max31865.autoConvert(true);

  // Set 50Hz noise filtering
  max31865.enable50Hz(true);

  console.log(sqINFO, "PTCMeasurement initialized successfully.");
}

/**
 * @brief Fetches the latest temperature measurement from the PTC sensor.
 */
void PTCMeasurement::getMeasurement()
{
  // Read the RTD value
  uint16_t rtdValue = max31865.readRTD();

  // Check for faults
  uint8_t fault = max31865.readFault();
  if (fault != 0)
  {
    console.log(sqINFO, "MAX31865 Fault: " + String(fault));
    max31865.clearFault(); // Clear the fault to proceed
    return;
  }

  // Calculate the temperature based on RTD and reference resistor values
  temperature = max31865.temperature(nominalResistance, referenceResistance);

  // Log the measured temperature
  console.log(sqINFO, "Measured Temperature: " + String(temperature) + " °C");
}

#endif // USE_MAX31865

#endif // USE_PTC
