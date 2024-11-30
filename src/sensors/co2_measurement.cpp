#pragma once
#include "bsp.h"
#ifdef USE_CO2
#include "siliqs_esp32.h"
#include "sensor_measurement.h"
#ifdef USE_DSCO2_UART
#include "SoftwareSerial.h"
#include "co2_measurement.h"

// 构造函数，使用指定的 RX 和 TX 引脚初始化 SoftwareSerial
Co2Measurement::Co2Measurement(uint8_t rxPin, uint8_t txPin) : softSerial(rxPin, txPin)
{
}

// 初始化方法，配置软件串口的波特率
void Co2Measurement::begin()
{
  softSerial.begin(DSCO2_BAUD_RATE); // 设置波特率为 9600
  busy = true;                       // 标记传感器正在工作
}

void Co2Measurement::end()
{
  softSerial.end(); // 关闭软件串口
  busy = false;     // 标记传感器空闲
}

// 获取 CO2 测量值
void Co2Measurement::getMeasurement()
{
  byte request[] = {0x42, 0x4d, 0xe3, 0x00, 0x00, 0x01, 0x72}; // 请求命令
  softSerial.write(request, sizeof(request));                  // 发送请求
  console.log(sqINFO, "Start");
  softSerial.flush(); // 确保所有数据已发送
  unsigned long startTime = millis();
  unsigned long timeout = 1000; // 1 秒超时

  // 等待传感器响应数据
  while (!softSerial.readBytes((byte *)&dsco2_data_v2, sizeof(dsco2_data_v2)))
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
  console.log(sqINFO, (uint8_t *)&dsco2_data_v2, sizeof(dsco2_data_v2));
  // 检查地址是否匹配
  if (memcmp((uint8_t *)&dsco2_data_v2, (uint8_t *)&request[0], sizeof(uint16_t)) != 0) // 使用 0x4D42 (BM) 作为预期的地址
  {
    console.log(sqINFO, "Address error");
    end();
    return;
  }

  // 交换字节顺序以符合传感器规范（视情况而定）
  for (int i = 0; i < sizeof(dsco2_data_v2) / sizeof(uint16_t); i++)
  {
    swap_bytes((uint16_t *)(&dsco2_data_v2.rawBytes[i]));
  }

  // 验证校验和
  if ((dsco2_data_v2.data.checksum - raw_sum((byte *)&dsco2_data_v2, sizeof(dsco2_data_v2))) == 0)
  {
    // CO2 数据有效，保存
    co2 = dsco2_data_v2.data.co2;
  }
  else
  {
    console.log(sqINFO, "Checksum error");
    end();
    return;
  }

  // 打印 CO2 值用于调试
  console.log(sqINFO, "CO2: %d ppm\n", dsco2_data_v2.data.co2);

  // 验证 CO2 范围并存储结果
  if (dsco2_data_v2.data.co2 < MIN_CO2 || dsco2_data_v2.data.co2 > MAX_CO2)
  {
    console.log(sqINFO, "CO2 out of range");
    co2 = -1;
    end();
    return;
  }

  busy = false;
}

// 析构函数实现
Co2Measurement::~Co2Measurement()
{
  console.log(sqINFO, "Destroying CO2 Sensor");
  end(); // 确保对象销毁时关闭串口
}

#endif
#endif