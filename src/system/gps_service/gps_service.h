// #pragma once
// #ifdef USE_GPS // Only compile when USE_GPS is enabled

// #ifndef GPS_SERVICE_H
// #define GPS_SERVICE_H

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include <Arduino.h>

// // Struct to store parsed GPS data
// typedef struct GPSData
// {
//   double latitude;    // Latitude
//   double longitude;   // Longitude
//   double altitude;    // Altitude in meters
//   double speed;       // Speed in km/h
//   double course;      // Course in degrees
//   uint8_t satellites; // Number of satellites
//   String utcTime;     // UTC Time (hh:mm:ss)
//   uint32_t timestamp; // Timestamp
//   bool valid;         // Data validity flag
// } GPSData;

// class GPSService
// {
// public:
//   GPSService(HardwareSerial &serialPort = Serial1, long baudRate = 9600, int rxPin = -1, int txPin = -1, int powerPin = -1);

//   // Initialize GPS service and start background task
//   bool begin();

//   // Stop background task and clean up
//   void stop();

//   // Retrieve the latest GPS data
//   GPSData getGPSData();

// private:
//   HardwareSerial &serial; // Reference to the HardwareSerial object
//   long baudRate;          // UART baud rate
//   int rxPin, txPin;       // UART pins
//   int powerPin;           // Power pin to enable/disable GPS module

//   static TaskHandle_t taskHandle;     // FreeRTOS task handle
//   static String gpsBuffer;            // Buffer for raw NMEA data
//   static GPSData gpsData;             // Parsed GPS data
//   static SemaphoreHandle_t dataMutex; // Mutex for data access

//   // Background task to read and parse GPS data
//   static void gpsTask(void *parameter);

//   // Parse incoming NMEA sentences
//   static void parseGPSData(const String &nmeaSentence);

//   // Specific NMEA parsers
//   static void parseGGA(const String &nmeaSentence);
//   static void parseRMC(const String &nmeaSentence);

//   // Helper functions for parsing
//   static void splitNMEA(const String &nmeaSentence, String *parts, int maxParts);
//   static double convertToDecimal(const String &raw, const String &direction);
//   static String formatUTCTime(const String &rawTime);
// };

// #endif // GPS_SERVICE_H
// #endif
