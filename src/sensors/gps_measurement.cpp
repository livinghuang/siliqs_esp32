#pragma once
#include "bsp.h"
#ifdef USE_GPS
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#include "gps_measurement.h"

GPSMeasurement::GPSMeasurement(uint8_t rxPin, uint8_t txPin)
    : rxPin(rxPin), txPin(txPin)
{
}

// 初始化方法，配置软件串口的波特率
void GPSMeasurement::begin()
{
  Serial1.begin(DEFAULT_BAUD_RATE, SERIAL_8N1, rxPin, txPin);
}

void GPSMeasurement::end()
{
  Serial1.end();
}

void GPSMeasurement::getRawData()
{
  if (Serial1.available() > 0)
  {                          // Check if data is available in the buffer
    char c = Serial1.read(); // Read one byte from Serial1
    Serial.print(c);
  }
}

bool GPSMeasurement::parseGPSData(const String &nmeaSentence)
{
  if (!nmeaSentence.startsWith("$"))
  {
    console.log(sqWARNING, "[ERROR] Invalid NMEA sentence: Does not start with '$'");
    return false;
  }

  if ((nmeaSentence.startsWith("$GPGGA")) || (nmeaSentence.startsWith("$GNGGA")))
  {
    if (parseGGA(nmeaSentence))
    {
      console.log(sqINFO, "lat:" + String(gpsData.latitude) + " lon:" + String(gpsData.longitude) + " alt:" + String(gpsData.altitude));
      return true;
    }
  }

  if (nmeaSentence.startsWith("$GPRMC") || (nmeaSentence.startsWith("$GNRMC")))
  {
    if (parseRMC(nmeaSentence))
    {
      console.log(sqINFO, "lat:" + String(gpsData.latitude) + " lon:" + String(gpsData.longitude));
      console.log(sqINFO, String(gpsData.speed) + " km/h, " + String(gpsData.course) + " degrees");
      console.log(sqINFO, "utcTime : " + gpsData.utcTime);
      return true;
    }
  }

  if (nmeaSentence.startsWith("$GNGLL"))
  {
    if (parseGLL(nmeaSentence))
    {
      console.log(sqINFO, "lat:" + String(gpsData.latitude) + " lon:" + String(gpsData.longitude));
      console.log(sqINFO, String(gpsData.speed) + " km/h, " + String(gpsData.course) + " degrees");
      console.log(sqINFO, "utcTime : " + gpsData.utcTime);
      return true;
    }
  }

  if (nmeaSentence.startsWith("$GNZDA"))
  {
    if (parseZDA(nmeaSentence))
    {
      console.log(sqINFO, "utcTime = " + gpsData.utcTime);
      return true;
    }
  }

  if (nmeaSentence.startsWith("$GNGSV") || nmeaSentence.startsWith("$BDGSV") || nmeaSentence.startsWith("$GPGSV"))
  {
    if (parseGSV(nmeaSentence))
    {
      console.log(sqINFO, "parseGSV success : satellites = " + String(gpsData.satellites));
      return true;
    }
  }
  if (nmeaSentence.startsWith("$GPTXT") || nmeaSentence.startsWith("$GNVTG") || nmeaSentence.startsWith("$GNGSA"))
  {
    return false;
  }

  // If no match is found, log the unknown sentence type
  console.log(sqWARNING, "Unknown NMEA sentence type:");
  console.log(sqWARNING, nmeaSentence);
  return false;
}

bool GPSMeasurement::parseZDA(const String &nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts, 15);
  if (parts[1].isEmpty() || parts[2].isEmpty() || parts[3].isEmpty() ||
      parts[4].isEmpty() || parts[5].isEmpty() || parts[6].isEmpty())
  {
    console.log(sqWARNING, "Missing critical ZDA fields");
    gpsData.valid = false;
  }
  else
  {
    gpsData.time.dd = parts[2].toInt();
    gpsData.time.mm = parts[3].toInt();
    gpsData.time.yy = parts[4].toInt();
    gpsData.time.utcOffset_hours = parts[5].toInt();
    gpsData.time.utcOffset_minutes = parts[6].toInt();
    formatUTCTime(parts[1]);
    gpsData.utcTime = String(gpsData.time.yy) + "/" +
                      String(gpsData.time.mm) + "/" +
                      String(gpsData.time.dd) + " T " +
                      String(gpsData.time.hr) + ":" +
                      String(gpsData.time.min) + ":" +
                      String(gpsData.time.sec) + " +" +
                      String(gpsData.time.utcOffset_hours) + ":" +
                      String(gpsData.time.utcOffset_minutes);
    gpsData.valid = true;
  }
  return true;
}

bool GPSMeasurement::parseGSV(const String &nmeaSentence)
{
  String parts[20];
  splitNMEA(nmeaSentence, parts, 20);
  if (parts[2].toInt() == 1) // First sentence
  {
    gpsData.satellites = parts[3].toInt();
    gpsData.valid = true;
    return true;
  }
  return false;
}

bool GPSMeasurement::parseGLL(const String &nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts, 15);
  if (parts[1].isEmpty() || parts[2].isEmpty() || parts[3].isEmpty() ||
      parts[4].isEmpty() || parts[5].isEmpty() || (parts[6] != "A"))
  {
    console.log(sqWARNING, "Missing critical GLL fields");
    gpsData.valid = false;
  }
  else
  {
    gpsData.latitude = convertToDecimal(parts[1], parts[2]);  // parts[1] = raw latitude, parts[2] = N/S
    gpsData.longitude = convertToDecimal(parts[3], parts[4]); // parts[3] = raw longitude, parts[4] = E/W
    gpsData.utcTime = formatUTCTime(parts[5]);
    gpsData.valid = false;
  }
  return true;
}

bool GPSMeasurement::parseGGA(const String &nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts, 15);
  if (parts[1].isEmpty() || parts[2].isEmpty() || parts[3].isEmpty() ||
      parts[4].isEmpty() || parts[5].isEmpty() || parts[9].isEmpty() || parts[7].isEmpty())
  {
    console.log(sqWARNING, "Missing critical GGA fields");
    gpsData.valid = false;
  }
  else
  {
    gpsData.utcTime = formatUTCTime(parts[1]);
    gpsData.latitude = convertToDecimal(parts[2], parts[3]);
    gpsData.longitude = convertToDecimal(parts[4], parts[5]);
    gpsData.altitude = parts[9].toDouble();
    gpsData.satellites = parts[7].toInt();
    gpsData.valid = true;
  }
  return gpsData.valid;
}

bool GPSMeasurement::parseRMC(const String &nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts, 15);
  if (parts[1].isEmpty() || (parts[2] != "A") || parts[3].isEmpty() || parts[4].isEmpty() ||
      parts[5].isEmpty() || parts[6].isEmpty())
  {
    console.log(sqWARNING, "Missing critical RMC fields");
    gpsData.valid = false;
  }
  else
  {
    gpsData.utcTime = formatUTCTime(parts[1]);
    gpsData.latitude = convertToDecimal(parts[3], parts[4]);
    gpsData.longitude = convertToDecimal(parts[5], parts[6]);
    gpsData.speed = parts[7].toDouble() * 1.852; // Convert knots to km/h
    gpsData.course = parts[8].toDouble();
    gpsData.valid = true;
  }
  return gpsData.valid;
}

void GPSMeasurement::splitNMEA(const String &nmeaSentence, String *parts, int maxParts)
{
  int index = 0, start = 0;
  for (int i = 0; i < nmeaSentence.length() && index < maxParts; ++i)
  {
    if (nmeaSentence[i] == ',' || nmeaSentence[i] == '*')
    {
      parts[index++] = nmeaSentence.substring(start, i);
      start = i + 1;
    }
  }
}

double GPSMeasurement::convertToDecimal(const String &raw, const String &direction)
{
  if (raw.isEmpty())
    return 0.0;

  double rawValue = raw.toDouble();
  int degrees = int(rawValue / 100);
  double minutes = rawValue - (degrees * 100);
  double decimal = degrees + (minutes / 60.0);

  if (direction == "S" || direction == "W")
  {
    decimal = -decimal;
  }

  return decimal;
}

String GPSMeasurement::formatUTCTime(const String &rawTime)
{
  if (rawTime.length() < 6)
    return "";

  String hours = rawTime.substring(0, 2);
  String minutes = rawTime.substring(2, 4);
  String seconds = rawTime.substring(4, 6);
  gpsData.time.hr = hours.toInt();
  gpsData.time.min = minutes.toInt();
  gpsData.time.sec = seconds.toInt();
  return hours + ":" + minutes + ":" + seconds;
}

// 获取 GPS 测量值
void GPSMeasurement::getMeasurement()
{
  while (Serial1.available() > 0)
  {
    char c = Serial1.read();
    if (c == '\n') // End of NMEA sentence
    {
      if (!gpsBuffer.isEmpty())
      {
        if (parseGPSData(gpsBuffer))
        {
          console.log(sqINFO, "Lat: %.2f Lon: %.2f Alt: %.2f", gpsData.latitude, gpsData.longitude, gpsData.altitude);
          console.log(sqINFO, "%02d/%02d/%02dT%02d:%02d:%02d+%02d:%02d", gpsData.time.yy, gpsData.time.mm, gpsData.time.dd, gpsData.time.hr, gpsData.time.min, gpsData.time.sec, gpsData.time.utcOffset_hours, gpsData.time.utcOffset_minutes);
        }
        gpsBuffer = ""; // Clear the buffer after processing
        break;
      }
    }
    else if (gpsBuffer.length() < 256) // Append character if within buffer limit
    {
      gpsBuffer += c;
    }
    else // Handle buffer overflow
    {
      console.log(sqWARNING, "GPS buffer overflow. Discarding data.");
      gpsBuffer = "";
    }
  }
  vTaskDelay(10 / portTICK_PERIOD_MS); // Ensure periodic execution
}

// 析构函数实现
GPSMeasurement::~GPSMeasurement()
{
  console.log(sqINFO, "Destroying GPS Sensor");
  end();
  stop();
}
#endif