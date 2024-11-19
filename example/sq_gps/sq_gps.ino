#include "bsp.h"
#include "siliqs_heltec_esp32.h"

void setup()
{
  // 初始化串口监视器
  Serial.begin(115200);
  Serial.println("Starting GPS Service Example...");

  // 启动GPS服务
  if (GPSService::begin(9600, GPS_RX, GPS_TX))
  {
    Serial.println("GPS Service started successfully.");
  }
  else
  {
    Serial.println("Failed to start GPS Service.");
  }
}

void loop()
{
  // 获取最新的GPS数据
  GPSData data = GPSService::getGPSData();

  // 打印GPS数据
  if (data.valid)
  {
    Serial.println("----- GPS Data -----");
    Serial.print("Latitude: ");
    Serial.println(data.latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(data.longitude, 6);
    Serial.print("Altitude: ");
    Serial.print(data.altitude);
    Serial.println(" m");
    Serial.print("Speed: ");
    Serial.print(data.speed);
    Serial.println(" km/h");
    Serial.print("Course: ");
    Serial.print(data.course);
    Serial.println(" degrees");
    Serial.print("Satellites: ");
    Serial.println(data.satellites);
    Serial.print("UTC Time: ");
    Serial.println(data.utcTime);
    Serial.println("---------------------");
  }
  else
  {
    Serial.println("No valid GPS data available.");
  }

  // 每隔1秒更新一次
  delay(1000);
}
