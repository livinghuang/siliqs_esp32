// #include "bsp.h"
// #ifdef USE_GPS // Only compile when USE_GPS is enabled
// #include "gps_service.h"

// // Static member initialization
// TaskHandle_t GPSService::taskHandle = nullptr;
// String GPSService::gpsBuffer = "";
// GPSData GPSService::gpsData = {0.0, 0.0, 0.0, 0.0, 0.0, 0, "", false};
// SemaphoreHandle_t GPSService::dataMutex = nullptr;

// GPSService::GPSService(HardwareSerial &serialPort, long baudRate, int rxPin, int txPin, int powerPin)
//     : serial(serialPort), baudRate(baudRate), rxPin(rxPin), txPin(txPin), powerPin(powerPin) {}

// bool GPSService::begin()
// {
//   if (powerPin != -1)
//   {
//     pinMode(powerPin, OUTPUT);
//     digitalWrite(powerPin, LOW); // Power on GPS module
//   }

//   serial.begin(baudRate, SERIAL_8N1, rxPin, txPin);

//   // Create mutex
//   dataMutex = xSemaphoreCreateMutex();
//   if (dataMutex == nullptr)
//   {
//     return false;
//   }

//   // Create FreeRTOS task
//   if (taskHandle == nullptr)
//   {
//     xTaskCreatePinnedToCore(gpsTask, "GPS Task", 4096, this, 1, &taskHandle, 1);
//   }

//   return true;
// }

// void GPSService::stop()
// {
//   if (taskHandle != nullptr)
//   {
//     vTaskDelete(taskHandle);
//     taskHandle = nullptr;
//   }

//   if (powerPin != -1)
//   {
//     digitalWrite(powerPin, LOW); // Power off GPS module
//   }

//   serial.end();

//   if (dataMutex != nullptr)
//   {
//     vSemaphoreDelete(dataMutex);
//     dataMutex = nullptr;
//   }
// }

// GPSData GPSService::getGPSData()
// {
//   xSemaphoreTake(dataMutex, portMAX_DELAY);
//   GPSData dataCopy = gpsData; // Create a copy of the data
//   xSemaphoreGive(dataMutex);
//   return dataCopy;
// }

// void GPSService::gpsTask(void *parameter)
// {
//   GPSService *instance = static_cast<GPSService *>(parameter);

//   while (true)
//   {
//     while (instance->serial.available())
//     {
//       char c = instance->serial.read();
//       if (c == '\n')
//       {
//         xSemaphoreTake(dataMutex, portMAX_DELAY);
//         parseGPSData(gpsBuffer);
//         gpsBuffer = "";
//         xSemaphoreGive(dataMutex);
//       }
//       else
//       {
//         gpsBuffer += c;
//       }
//     }
//     vTaskDelay(10 / portTICK_PERIOD_MS);
//   }
// }

// void GPSService::parseGPSData(const String &nmeaSentence)
// {
//   if (nmeaSentence.startsWith("$GPGGA"))
//   {
//     parseGGA(nmeaSentence);
//   }
//   else if (nmeaSentence.startsWith("$GPRMC"))
//   {
//     parseRMC(nmeaSentence);
//   }
// }

// void GPSService::parseGGA(const String &nmeaSentence)
// {
//   String parts[15];
//   splitNMEA(nmeaSentence, parts, 15);

//   gpsData.utcTime = formatUTCTime(parts[1]);
//   gpsData.latitude = convertToDecimal(parts[2], parts[3]);
//   gpsData.longitude = convertToDecimal(parts[4], parts[5]);
//   gpsData.altitude = parts[9].toDouble();
//   gpsData.satellites = parts[7].toInt();
//   gpsData.valid = !parts[6].isEmpty() && parts[6] != "0";
// }

// void GPSService::parseRMC(const String &nmeaSentence)
// {
//   String parts[15];
//   splitNMEA(nmeaSentence, parts, 15);

//   gpsData.utcTime = formatUTCTime(parts[1]);
//   gpsData.latitude = convertToDecimal(parts[3], parts[4]);
//   gpsData.longitude = convertToDecimal(parts[5], parts[6]);
//   gpsData.speed = parts[7].toDouble() * 1.852; // Convert knots to km/h
//   gpsData.course = parts[8].toDouble();
//   gpsData.valid = parts[2] == "A";
// }

// void GPSService::splitNMEA(const String &nmeaSentence, String *parts, int maxParts)
// {
//   int index = 0, start = 0;
//   for (int i = 0; i < nmeaSentence.length() && index < maxParts; ++i)
//   {
//     if (nmeaSentence[i] == ',' || nmeaSentence[i] == '*')
//     {
//       parts[index++] = nmeaSentence.substring(start, i);
//       start = i + 1;
//     }
//   }
// }

// double GPSService::convertToDecimal(const String &raw, const String &direction)
// {
//   if (raw.isEmpty())
//     return 0.0;

//   double rawValue = raw.toDouble();
//   int degrees = int(rawValue / 100);
//   double minutes = rawValue - (degrees * 100);
//   double decimal = degrees + (minutes / 60.0);

//   if (direction == "S" || direction == "W")
//   {
//     decimal = -decimal;
//   }

//   return decimal;
// }

// String GPSService::formatUTCTime(const String &rawTime)
// {
//   if (rawTime.length() < 6)
//     return "";

//   String hours = rawTime.substring(0, 2);
//   String minutes = rawTime.substring(2, 4);
//   String seconds = rawTime.substring(4, 6);

//   return hours + ":" + minutes + ":" + seconds;
// }
// #endif