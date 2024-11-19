#include "bsp.h"
#ifdef USE_GPS // Only compile when USE_GPS is enabled
#include "gps_service.h"

// 初始化静态成员变量
TaskHandle_t GPSService::taskHandle = nullptr;
String GPSService::gpsBuffer = "";
GPSData GPSService::gpsData = {0.0, 0.0, 0.0, 0.0, 0.0, 0, "", false};
SemaphoreHandle_t GPSService::dataMutex = nullptr;

bool GPSService::begin(long baudRate, int rxPin, int txPin)
{
  // 初始化GPS串口
  Serial1.begin(baudRate, SERIAL_8N1, rxPin, txPin);
  Serial.println("GPS module initialized.");

  // 创建互斥锁
  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == nullptr)
  {
    Serial.println("Failed to create mutex.");
    return false;
  }

  // 创建并启动GPS后台任务
  if (xTaskCreate(gpsTask, "GPSTask", 4096, nullptr, 1, &taskHandle) != pdPASS)
  {
    Serial.println("Failed to create GPS task.");
    return false;
  }

  return true;
}

void GPSService::stop()
{
  if (taskHandle != nullptr)
  {
    vTaskDelete(taskHandle);
    taskHandle = nullptr;
  }

  if (dataMutex != nullptr)
  {
    vSemaphoreDelete(dataMutex);
    dataMutex = nullptr;
  }

  Serial.println("GPS service stopped.");
}

GPSData GPSService::getGPSData()
{
  GPSData dataCopy;

  // 获取互斥锁并复制数据
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
  {
    dataCopy = gpsData;
    xSemaphoreGive(dataMutex);
  }

  return dataCopy;
}

void GPSService::gpsTask(void *parameter)
{
  while (true)
  {
    while (Serial1.available() > 0)
    {
      char c = Serial1.read();
      if (c == '\n') // 完整NMEA语句结束
      {
        parseGPSData(gpsBuffer);
        gpsBuffer = ""; // 清空缓冲区
      }
      else if (c != '\r')
      {
        gpsBuffer += c; // 追加字符到缓冲区
      }
    }

    // 延迟以减少CPU占用
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void GPSService::parseGPSData(String nmeaSentence)
{
  if (nmeaSentence.startsWith("$GNGGA"))
  {
    parseGGA(nmeaSentence);
  }
  else if (nmeaSentence.startsWith("$GNRMC"))
  {
    parseRMC(nmeaSentence);
  }
}

void GPSService::parseGGA(String nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts);

  if (parts[6].toInt() > 0) // 检查是否有有效定位
  {
    double lat = convertToDecimal(parts[2], parts[3]);
    double lon = convertToDecimal(parts[4], parts[5]);
    double alt = parts[9].toDouble();
    uint8_t sats = parts[7].toInt();

    // 更新数据
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
      gpsData.latitude = lat;
      gpsData.longitude = lon;
      gpsData.altitude = alt;
      gpsData.satellites = sats;
      gpsData.valid = true;
      xSemaphoreGive(dataMutex);
    }

    Serial.println("GGA Data Updated.");
  }
}

void GPSService::parseRMC(String nmeaSentence)
{
  String parts[15];
  splitNMEA(nmeaSentence, parts);

  if (parts[2] == "A") // 检查是否有有效定位
  {
    double lat = convertToDecimal(parts[3], parts[4]);
    double lon = convertToDecimal(parts[5], parts[6]);
    double speed = parts[7].toDouble() * 1.852; // 转换为km/h
    double course = parts[8].toDouble();
    String utcTime = formatUTCTime(parts[1]);

    // 更新数据
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
      gpsData.latitude = lat;
      gpsData.longitude = lon;
      gpsData.speed = speed;
      gpsData.course = course;
      gpsData.utcTime = utcTime;
      gpsData.valid = true;
      xSemaphoreGive(dataMutex);
    }

    Serial.println("RMC Data Updated.");
  }
}

void GPSService::splitNMEA(String nmeaSentence, String *parts)
{
  int index = 0;
  int lastIndex = 0;

  for (int i = 0; i < nmeaSentence.length(); i++)
  {
    if (nmeaSentence.charAt(i) == ',' || i == nmeaSentence.length() - 1)
    {
      parts[index] = nmeaSentence.substring(lastIndex, i);
      lastIndex = i + 1;
      index++;
    }
  }
}

double GPSService::convertToDecimal(String raw, String direction)
{
  double degrees = raw.substring(0, raw.indexOf('.') - 2).toDouble();
  double minutes = raw.substring(raw.indexOf('.') - 2).toDouble();
  double decimal = degrees + (minutes / 60.0);

  if (direction == "S" || direction == "W")
    decimal = -decimal;

  return decimal;
}

String GPSService::formatUTCTime(String rawTime)
{
  if (rawTime.length() < 6)
    return "";

  String hours = rawTime.substring(0, 2);
  String minutes = rawTime.substring(2, 4);
  String seconds = rawTime.substring(4, 6);

  return hours + ":" + minutes + ":" + seconds;
}
#endif