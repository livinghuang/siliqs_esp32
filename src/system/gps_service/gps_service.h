#include "bsp.h"
#pragma once
#ifdef USE_GPS // Only compile when USE_GPS is enabled

#ifndef GPS_SERVICE_H
#define GPS_SERVICE_H

#include "siliqs_esp32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

// 定义结构体以存储解析后的GPS数据
typedef struct GPSData
{
  double latitude;    // 纬度
  double longitude;   // 经度
  double altitude;    // 海拔高度
  double speed;       // 速度（km/h）
  double course;      // 航向（度）
  uint8_t satellites; // 捕获的卫星数量
  String utcTime;     // UTC时间 (hh:mm:ss)
  bool valid;         // 数据是否有效
} GPSData;

class GPSService
{
public:
  // 初始化GPS服务，并启动后台任务
  static bool begin(long baudRate = 9600, int rxPin = GPS_RX, int txPin = GPS_TX);

  // 停止后台任务
  static void stop();

  // 获取最新的GPS数据
  static GPSData getGPSData();

private:
  // FreeRTOS任务句柄
  static TaskHandle_t taskHandle;

  // NMEA数据缓冲区
  static String gpsBuffer;

  // 存储解析后的GPS数据
  static GPSData gpsData;

  // GPS数据互斥锁
  static SemaphoreHandle_t dataMutex;

  // GPS后台任务函数
  static void gpsTask(void *parameter);

  // 解析NMEA语句
  static void parseGPSData(String nmeaSentence);

  // 解析具体NMEA语句
  static void parseGGA(String nmeaSentence);
  static void parseRMC(String nmeaSentence);

  // 辅助函数
  static void splitNMEA(String nmeaSentence, String *parts);
  static double convertToDecimal(String raw, String direction);
  static String formatUTCTime(String rawTime); // 格式化UTC时间
};

#endif // GPS_SERVICE_H
#endif