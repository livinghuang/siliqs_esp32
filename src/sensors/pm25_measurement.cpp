#pragma once
#include "bsp.h"
#ifdef USE_PM25
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#ifdef USE_G5T_UART
#include "SoftwareSerial.h"
#include "pm25_measurement.h"
// 构造函数，使用指定的 RX 和 TX 引脚初始化 SoftwareSerial
PM25Measurement::PM25Measurement(uint8_t rxPin, uint8_t txPin) : softSerial(rxPin, txPin)
{
}

// 初始化方法，配置软件串口的波特率
void PM25Measurement::begin()
{
  softSerial.begin(G5T_BAUD_RATE); // 设置波特率为 9600
  busy = true;                     // 标记传感器正在工作
}

void PM25Measurement::end()
{
  softSerial.end(); // 关闭软件串口
  busy = false;     // 标记传感器空闲
}

// 获取 PM2.5 和 PM10 测量值
void PM25Measurement::getMeasurement()
{
  byte request[] = {0x42, 0x4d, 0xe3, 0x00, 0x00, 0x01, 0x72};
  softSerial.write(request, sizeof(request)); // 发送请求
  console.log(sqINFO, "Start");
  softSerial.flush(); // 确保所有数据已发送
  unsigned long startTime = millis();
  unsigned long timeout = 1000; // 1 秒超时

  // 等待传感器响应数据
  while (!softSerial.readBytes((byte *)&g5t_data_v2, sizeof(g5t_data_v2)))
  {
    if (millis() - startTime >= timeout)
    {
      console.log(sqINFO, "Timeout occurred!");
      end(); // 超时后关闭串口
      return;
    }
  }

  softSerial.end(); // 读取完成后关闭串口

  // 打印接收到的十六进制数据用于调试
  console.log(sqINFO, (uint8_t *)&g5t_data_v2, sizeof(g5t_data_v2));

  // 检查地址是否匹配
  if (memcmp((uint8_t *)&g5t_data_v2.data.address, (uint8_t *)&request[0], sizeof(uint16_t)) != 0) // 使用 0x4D42 (BM) 作为预期的地址
  {
    console.log(sqINFO, "Address error");
    end();
    return;
  }

  // 交换字节顺序以符合传感器规范（视情况而定）
  for (int i = 0; i < sizeof(g5t_data_v2) / sizeof(uint16_t); i++)
  {
    swap_bytes((uint16_t *)(&g5t_data_v2.rawBytes[i]));
  }

  // 验证校验和
  if ((g5t_data_v2.data.checksum - raw_sum((byte *)&g5t_data_v2, sizeof(g5t_data_v2))) == 0)
  {
    // PM2.5 和 PM10 数据有效，保存
    pm2p5 = g5t_data_v2.data.pm2p5;
    pm10 = g5t_data_v2.data.pm10;
  }
  else
  {
    console.log(sqINFO, "Checksum error");
    end();
    return;
  }

  // 打印 PM2.5 和 PM10 值用于调试
  console.log(sqINFO, "PM2.5: %d μg/m³", pm2p5);
  console.log(sqINFO, "PM10: %d μg/m³", pm10);

  // 验证 PM2.5 范围并存储结果
  if (pm2p5 < MIN_PM2P5 || pm2p5 > MAX_PM2P5)
  {
    console.log(sqINFO, "PM2.5 out of range");
    pm2p5 = -1;
  }

  // 验证 PM10 范围并存储结果
  if (pm10 < MIN_PM10 || pm10 > MAX_PM10)
  {
    console.log(sqINFO, "PM10 out of range");
    pm10 = -1;
  }

  busy = false; // 标记任务完成
}

// 析构函数实现
PM25Measurement::~PM25Measurement()
{
  console.log(sqINFO, "Destroying PM25 Sensor");
  end(); // 确保对象销毁时关闭串口
}

#endif
#endif