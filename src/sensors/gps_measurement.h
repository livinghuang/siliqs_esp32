#pragma once
#include "bsp.h"
#ifdef USE_GPS
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#include <map>
// Struct to store parsed GPS data

struct dateTime
{
  int yy;
  int mm;
  int dd;
  int hr;
  int min;
  int sec;
  int utcOffset_hours;
  int utcOffset_minutes;
};

typedef struct GPSData
{
  double latitude;    // Latitude
  double longitude;   // Longitude
  double altitude;    // Altitude in meters
  double speed;       // Speed in km/h
  double course;      // Course in degrees
  uint8_t satellites; // Number of satellites
  struct dateTime time;
  String utcTime; // UTC Time (hh:mm:ss)
  bool valid;     // Data validity flag
} GPSData;

class GPSMeasurement : public Sensor
{
public:
  static constexpr int DEFAULT_BAUD_RATE = 9600;

  GPSMeasurement(uint8_t rxPin, uint8_t txPin);
  ~GPSMeasurement();

  void begin() override; // Initialize the sensor and start the task
  void end();            // end the serial port
  void getMeasurement() override;
  void getRawData();
  GPSData gpsData; // internal copy
private:
  String gpsBuffer = "";
  uint8_t rxPin; // RX pin for GPS communication
  uint8_t txPin; // TX pin for GPS communication
                 // Parse incoming NMEA sentences
  bool parseGPSData(const String &nmeaSentence);
  // Specific NMEA parsers
  bool parseGGA(const String &nmeaSentence);
  bool parseRMC(const String &nmeaSentence);
  bool parseGLL(const String &nmeaSentence);
  bool parseGSV(const String &nmeaSentence);
  bool parseZDA(const String &nmeaSentence);
  // Helper functions for parsing
  void splitNMEA(const String &nmeaSentence, String *parts, int maxParts);
  double convertToDecimal(const String &raw, const String &direction);
  String formatUTCTime(const String &rawTime);
};

#endif
