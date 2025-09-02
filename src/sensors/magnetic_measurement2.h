#pragma once
#include "bsp.h"
#ifdef USE_MAGNETOMETER2
#include "Arduino.h"
#include <Wire.h>

#define QMC5883_ADDRESS 0x2C
#define QMC5883_REG_0A 0x0A
#define QMC5883_REG_0B 0x0B

enum QMC5883_MODE
{
  QMC5883_SUSPEND = 0,
  QMC5883_NORMAL = 1,
  QMC5883_SINGLE = 2,
  QMC5883_CONTINUOUS = 3
};
enum QMC5883_ODR
{
  QMC5883_10HZ = 0,
  QMC5883_50HZ = 1,
  QMC5883_100HZ = 2,
  QMC5883_200HZ = 3
};
enum QMC5883_BW_FILTER
{
  QMC5883_BW_8 = 0,
  QMC5883_BW_4 = 1,
  QMC5883_BW_2 = 2,
  QMC5883_BW_1 = 3
};
enum QMC5883_NOISE_FILTER
{
  QMC5883_NF_1 = 0,
  QMC5883_NF_2 = 1,
  QMC5883_NF_4 = 2,
  QMC5883_NF_8 = 3
};
enum QMC5883_SET_RESET_MODE
{
  QMC5883_SET_RESET_ON = 0,
  QMC5883_SET_ONLY_ON = 1,
  QMC5883_SET_RESET_OFF = 2,
  QMC5883_SET_RESET_OFF1 = 3
};
enum QMC5883_RNG
{
  QMC5883_RNG_30G = 0,
  QMC5883_RNG_12G = 1,
  QMC5883_RNG_8G = 2,
  QMC5883_RNG_2G = 3
};
enum QMC5883_SELF_TEST
{
  QMC5883_SELF_TEST_OFF = 0,
  QMC5883_SELF_TEST_ON = 1
};
enum QMC5883_SOFT_RESET
{
  QMC5883_SOFT_RESET_OFF = 0,
  QMC5883_SOFT_RESET_ON = 1
};

class Magnetometer2
{
public:
  QMC5883_MODE mode = QMC5883_CONTINUOUS;
  QMC5883_ODR odr = QMC5883_100HZ;
  QMC5883_BW_FILTER bw = QMC5883_BW_8;
  QMC5883_NOISE_FILTER nf = QMC5883_NF_2;
  QMC5883_SOFT_RESET soft_reset = QMC5883_SOFT_RESET_OFF;
  QMC5883_SELF_TEST self_test = QMC5883_SELF_TEST_OFF;
  QMC5883_RNG rng = QMC5883_RNG_30G;
  QMC5883_SET_RESET_MODE set_reset_mode = QMC5883_SET_RESET_ON;
  // 報警旗標
  bool alarmOverflow = false;
  bool alarmCommError = false;
  unsigned long lastAlarmTime = 0;
  int samples = 4;
  int16_t x = 0, y = 0, z = 0;
  float gaussX = 0, gaussY = 0, gaussZ = 0;
  int mGaussX = 0, mGaussY = 0, mGaussZ = 0;

  Magnetometer2(uint8_t addr = QMC5883_ADDRESS, int sda = SDA, int scl = SCL)
      : address(addr), sdaPin(sda), sclPin(scl), _wire(nullptr) {}

  bool begin(TwoWire &wirePort = Wire, uint32_t i2cFreq = 400000)
  {
    _wire = &wirePort;
    _wire->begin(sdaPin, sclPin, i2cFreq);
    delay(2);
    setInitialConfig();
    writeRegisters();
    delay(2);
    return available();
  }
  void autoRange()
  {
    int curIdx = rng;

    // 先確認溢位
    if (isOverflow() && curIdx > 0)
    {
      setRange((QMC5883_RNG)(curIdx - 1)); // 切到更寬的量測範圍
      Serial.printf("Overflow! Auto-Range UP: Switch to %d G\n", getRangeGauss((QMC5883_RNG)(curIdx - 1)));
      // 建議重新量測一次，否則那筆值是 overflow 的
      return;
    }

    // 沒有溢位才看數值是否該自動往更高靈敏度切換
    static const int16_t LSB_MAX[] = {1000 * 30, 2500 * 12, 3750 * 8, 15000 * 2};
    int16_t absX = abs(x), absY = abs(y), absZ = abs(z);
    bool shouldDown = (absX < LSB_MAX[curIdx] * 0.3 && absY < LSB_MAX[curIdx] * 0.3 && absZ < LSB_MAX[curIdx] * 0.3);

    if (shouldDown && curIdx < 3)
    {
      setRange((QMC5883_RNG)(curIdx + 1)); // 回到更靈敏的範圍
      Serial.printf("Auto-Range DOWN: Switch to %d G\n", getRangeGauss((QMC5883_RNG)(curIdx + 1)));
    }
  }

  void setRange(QMC5883_RNG new_rng)
  {
    rng = new_rng;
    setInitialConfig();
    writeRegisters();
  }
  bool isOverflow()
  {
    _wire->beginTransmission(address);
    _wire->write(0x09); // Status register
    _wire->endTransmission(false);
    _wire->requestFrom(address, 1);
    if (_wire->available())
    {
      uint8_t status = _wire->read();
      return status & 0x02; // OVFL = bit 1
    }
    return false;
  }

  int getRangeGauss(QMC5883_RNG rng)
  {
    static const int LUT[] = {30, 12, 8, 2};
    return LUT[rng];
  }

  void getMeasurement()
  {
    alarmOverflow = false;
    alarmCommError = false;
    if (!_wire)
    {
      alarmCommError = true;
      alarm("I2C wire not initialized");
      return;
    }

    int32_t sx = 0, sy = 0, sz = 0, count = 0;
    for (int i = 0; i < samples; ++i)
    {
      int16_t rx, ry, rz;
      if (!readRaw(rx, ry, rz))
      {
        alarmCommError = true;
        alarm("I2C read failed");
        return;
      }
      sx += rx;
      sy += ry;
      sz += rz;
      count++;
      delayMicroseconds(1000000 / odrToHz());
    }
    if (count)
    {
      x = sx / count;
      y = sy / count;
      z = sz / count;
      convertToGauss();
    }

    // 檢查 overflow
    if (isOverflow())
    {
      alarmOverflow = true;
      alarm("Magnetometer overflow");
      // 可自動 auto-range 或通知主程式
    }
  }
  void alarm(const char *msg)
  {
    // OLED 顯示警告
    Serial.print("[ALARM] ");
    Serial.println(msg);
    lastAlarmTime = millis();
    // 也可控制蜂鳴器/LED/MQTT 發送
  }

  // 外部可查詢報警狀態
  bool isAlarm() { return alarmOverflow || alarmCommError; }
  void print()
  {
    Serial.printf("Magnetic: X: %d Y: %d Z: %d (LSB)\n", x, y, z);
    Serial.printf("Gauss: X: %.3f Y: %.3f Z: %.3f\n", gaussX, gaussY, gaussZ);
  }

  bool available()
  {
    _wire->beginTransmission(address);
    _wire->write(0x0D); // WHO_AM_I
    _wire->endTransmission(false);
    _wire->requestFrom(address, 1);
    if (_wire->available())
      return _wire->read() == 0xFF; // QMC5883P 回傳 0xFF
    return false;
  }

private:
  uint8_t address;
  int sdaPin, sclPin;
  TwoWire *_wire;
  uint8_t configA = 0, configB = 0;

  void setInitialConfig()
  {
    configA = (nf << 6) | (bw << 4) | (odr << 2) | (mode);
    configB = (soft_reset << 7) | (self_test << 6) | (rng << 2) | (set_reset_mode);
  }
  void writeRegisters()
  {
    writeRegister(QMC5883_REG_0B, configB);
    delay(2);
    writeRegister(QMC5883_REG_0A, configA);
    delay(2);
  }
  void writeRegister(uint8_t reg, uint8_t val)
  {
    _wire->beginTransmission(address);
    _wire->write(reg);
    _wire->write(val);
    _wire->endTransmission();
  }
  bool readRaw(int16_t &rx, int16_t &ry, int16_t &rz)
  {
    _wire->beginTransmission(address);
    _wire->write(0x01); // data output X_LSB
    if (_wire->endTransmission(false) != 0)
      return false;
    if (_wire->requestFrom(address, (uint8_t)6) != 6)
      return false;
    uint8_t xl = _wire->read(), xh = _wire->read();
    uint8_t yl = _wire->read(), yh = _wire->read();
    uint8_t zl = _wire->read(), zh = _wire->read();
    rx = (int16_t)(xh << 8 | xl);
    ry = (int16_t)(yh << 8 | yl);
    rz = (int16_t)(zh << 8 | zl);
    return true;
  }
  void convertToGauss()
  {
    static const float LSB_PER_G[] = {1000.0, 2500.0, 3750.0, 15000.0};
    float sens = LSB_PER_G[rng];
    gaussX = x / sens;
    gaussY = y / sens;
    gaussZ = z / sens;
    mGaussX = gaussX * 1000;
    mGaussY = gaussY * 1000;
    mGaussZ = gaussZ * 1000;
  }
  uint8_t odrToHz() const
  {
    static const uint8_t LUT[] = {10, 50, 100, 200};
    return LUT[odr];
  }
};
extern Magnetometer2 mag;

void test_magnetic();
// #define GAIN_LSB_TO_30G_00 1000 // +/- 30G sensitivity = 1000 LSB/Gauss
// #define GAIN_LSB_TO_12G_10 2500 // +/- 12G sensitivity = 2500 LSB/Gauss
// #define GAIN_LSB_TO_8G_20 3750  // +/- 8G sensitivity = 3750 LSB/Gauss
// #define GAIN_LSB_TO_2G_30 15000 // +/- 2G sensitivity = 15000 LSB/Gauss

// #define QMC5883_ADDRESS 0x2C
// #define QMC5883_REG_0A 0x0A
// #define QMC5883_REG_0B 0x0B

// /*
// Control register 1 is located in address 0AH, it sets the operational modes (MODE) and over sampling rate (OSR).
// Control register 2 is located in address 0BH. It controls soft reset, self-test and set/reset mode.
// Two bits of MODE registers can transfer mode of operations in the device, the four modes are Suspend Mode,
// Normal mode, Single Mode and Continuous Mode. The default mode after Power-On-Reset (POR) is Suspend
// Mode. Suspend Mode should be added in the middle of mode shifting between Continuous Mode、Single Mode
// and Normal Mode.
// */
// enum QMC5883_MODE
// {
//   QMC5883_SUSPEND = 0b00,
//   QMC5883_NORMAL = 0b01,
//   QMC5883_SINGLE = 0b10,
//   QMC5883_CONTINUOUS = 0b11,
// };

// /*
// The Output data rate is controlled by ODR registers. Four data update frequencies can be selected: 10Hz, 50Hz,
// 100Hz or 200Hz
// */
// enum QMC5883_ODR
// {
//   QMC5883_10HZ = 0b00,
//   QMC5883_50HZ = 0b01,
//   QMC5883_100HZ = 0b10,
//   QMC5883_200HZ = 0b11,
// };

// /*
// Over sample Rate (OSR1) registers are used to control bandwidth of an internal digital filter. Larger OSR value
// leads to smaller filter bandwidth, less in-band noise and higher power consumption. It could be used to reach a
// good balance between noise and power. Four over sample ratios can be selected, 8,4,2 or 1.
// */
// enum QMC5883_BW_FILTER
// {
//   QMC5883_BW_8 = 0b00,
//   QMC5883_BW_4 = 0b01,
//   QMC5883_BW_2 = 0b10,
//   QMC5883_BW_1 = 0b11,
// };

// /*
// Another filter is added for better noise performance; the depth can be adjusted through OSR2.
// */
// enum QMC5883_NOISE_FILTER
// {
//   QMC5883_NF_1 = 0b00,
//   QMC5883_NF_2 = 0b01,
//   QMC5883_NF_4 = 0b10,
//   QMC5883_NF_8 = 0b11,
// };

// /*
// Set/Reset Mode can be control by the register SET/RESET MODE. There are 3 modes for selection：SET AND
// RESET ON， SET ONLY ON and SET AND RESET OFF. In SET ONLY ON or SET AND RESET OFF mode, the
// offset is not renewed during measuring.
// BIT 1 BIT 0
// */
// enum QMC5883_SET_RESET_MODE
// {
//   QMC5883_SET_RESET_ON = 0b00,
//   QMC5883_SET_ONLY_ON = 0b01,
//   QMC5883_SET_RESET_OFF = 0b10,
//   QMC5883_SET_RESET_OFF1 = 0b11,
// };

// /*
// Field ranges of the magnetic sensor can be selected through the register RNG. The full-scale range is determined
// by the application environments. The lowest field range has the highest sensitivity, therefore, higher resolution.
// BIT 3 BIT 2
// */
// enum QMC5883_RNG
// {
//   QMC5883_RNG_30G = 0b00,
//   QMC5883_RNG_12G = 0b01,
//   QMC5883_RNG_8G = 0b10,
//   QMC5883_RNG_2G = 0b11,
// };

// /*
// 1: self_test enable, auto clear after the data is updated
// BIT 6
// */
// enum QMC5883_SELF_TEST
// {
//   QMC5883_SELF_TEST_ON = 0b01,
//   QMC5883_SELF_TEST_OFF = 0b00,
// };
// /*
// 1：Soft reset, restore default value of all registers，0: no reset
// BIT 7
// */
// enum QMC5883_SOFT_RESET
// {
//   QMC5883_SOFT_RESET_ON = 0b01,
//   QMC5883_SOFT_RESET_OFF = 0b00,
// };

// struct QMC5883_registers
// {
//   uint8_t RegA;
//   uint8_t RegB;
// };

// class Magnetometer2 : public Sensor
// {
// private:
//   QMC5883_registers config;
//   uint8_t address;

// public:
//   QMC5883_MODE mode = QMC5883_CONTINUOUS;
//   QMC5883_ODR odr = QMC5883_100HZ;
//   QMC5883_BW_FILTER bw = QMC5883_BW_1;
//   QMC5883_NOISE_FILTER nf = QMC5883_NF_2;
//   QMC5883_SOFT_RESET soft_reset = QMC5883_SOFT_RESET_OFF;
//   QMC5883_SELF_TEST self_test = QMC5883_SELF_TEST_OFF;
//   QMC5883_RNG rng = QMC5883_RNG_2G;
//   QMC5883_SET_RESET_MODE set_reset_mode = QMC5883_SET_RESET_ON;
//   int16_t x = 0, y = 0, z = 0; // Public access to store measurements
//   float gaussX, gaussY, gaussZ;
//   float mGaussX, mGaussY, mGaussZ;
//   int samples = 4;
//   /**
//    * @brief Constructor for the Magnetometer class
//    * @param address I2C address of the QMC5883
//    * @param sda SDA pin
//    * @param scl SCL pin
//    */
//   Magnetometer2(uint8_t address = QMC5883_ADDRESS, int sda = pMQC5883_I2C_SDA, int scl = pMQC5883_I2C_SCL)
//   {
//     mode = QMC5883_CONTINUOUS;
//     odr = QMC5883_100HZ;
//     bw = QMC5883_BW_8;
//     nf = QMC5883_NF_2;
//     soft_reset = QMC5883_SOFT_RESET_OFF;
//     self_test = QMC5883_SELF_TEST_OFF;
//     rng = QMC5883_RNG_30G;
//     set_reset_mode = QMC5883_SET_RESET_ON;
//   }

//   /**
//    * @brief Initialize the magnetometer
//    */
//   void begin() override
//   {
//     Wire.begin(pMQC5883_I2C_SDA, pMQC5883_I2C_SCL, 100000);
//     address = QMC5883_ADDRESS; // Set internal address member
//     setInitialConfig();
//     Serial.println("Set Initial QMC5883 registers");
//     write_configurations_to_registers();
//   }

//   /**
//    * @brief Get the magnetic field measurements
//    */
//   void getMeasurement() override
//   {
//     long sumX = 0, sumY = 0, sumZ = 0;
//     int fetch_times = 0;
//     for (int i = 0; i < samples; i++)
//     {
//       Wire.beginTransmission(QMC5883_ADDRESS);
//       Wire.write(0x01);            // 設定讀取寄存器
//       Wire.endTransmission(false); // false 保持總線不結束
//       Wire.requestFrom(QMC5883_ADDRESS, 6);
//       vTaskDelay(pdMS_TO_TICKS(10)); // 避免過快讀取，可以根據實際情況調整
//       int16_t a = 0, b = 0, c = 0;
//       if (Wire.available() == 6)
//       {
//         get_raw(&a, &b, &c);
//         sumX += a;
//         sumY += b;
//         sumZ += c;
//         fetch_times++;
//       }
//       vTaskDelay(pdMS_TO_TICKS(10)); // 避免過快讀取，可以根據實際情況調整
//     }
//     x = sumX / fetch_times;
//     y = sumY / fetch_times;
//     z = sumZ / fetch_times;
//     get_gauss();
//   }

//   ~Magnetometer2() override {} // Virtual destructor

//   void setControlRegisterA(QMC5883_MODE mode, QMC5883_ODR odr, QMC5883_BW_FILTER bw, QMC5883_NOISE_FILTER nf)
//   {
//     config.RegA = mode << 6 | odr << 4 | bw << 2 | nf;
//   }
//   void setControlRegisterB(QMC5883_SOFT_RESET soft_reset, QMC5883_SELF_TEST self_test, QMC5883_RNG rng, QMC5883_SET_RESET_MODE set_reset_mode)
//   {
//     config.RegB = soft_reset << 7 | self_test << 6 | rng << 2 | set_reset_mode;
//   }
//   void write_configurations_to_registers()
//   {
//     writeRegister(QMC5883_REG_0B, config.RegB);
//     // print_hex((uint8_t *)&config.RegB, sizeof(config.RegB));
//     delay(10);
//     writeRegister(QMC5883_REG_0A, config.RegA);
//     // print_hex((uint8_t *)&config.RegA, sizeof(config.RegA));
//   }

//   void print()
//   {
//     Serial.println("Magnetic: ");
//     Serial.printf("X: %d LSB, Y: %d LSB, Z: %d LSB\n", x, y, z);
//     Serial.printf("gX: %f GAUSS, Y: %f GAUSS, Z: %f GAUSS\n", gaussX, gaussY, gaussZ);
//   }

//   bool available()
//   {
//     Wire.beginTransmission(QMC5883_ADDRESS);
//     Wire.write(0x00);
//     Wire.endTransmission(false);
//     Wire.requestFrom(QMC5883_ADDRESS, 1);
//     if (Wire.available())
//     {
//       uint8_t chipId = Wire.read();
//       return chipId == 0x80; // 依 datasheet 預設值
//     }
//     return false;
//   }

//   void setInitialConfig()
//   {
//     setControlRegisterA(mode, odr, bw, nf);
//     setControlRegisterB(soft_reset, self_test, rng, set_reset_mode);
//   }

//   void get_data(int16_t *x, int16_t *y, int16_t *z, int samples = 4)
//   {
//     long sumX = 0, sumY = 0, sumZ = 0;

//     for (int i = 0; i < samples; i++)
//     {
//       Wire.beginTransmission(QMC5883_ADDRESS);
//       Wire.write(0x01);            // 設定讀取寄存器
//       Wire.endTransmission(false); // false 保持總線不結束
//       Wire.requestFrom(QMC5883_ADDRESS, 6);

//       if (Wire.available() == 6)
//       {
//         uint8_t xL = Wire.read();
//         uint8_t xH = Wire.read();

//         uint8_t yL = Wire.read();
//         uint8_t yH = Wire.read();

//         uint8_t zL = Wire.read();
//         uint8_t zH = Wire.read();

//         int16_t rawX = (int16_t)(xH << 8 | xL);
//         int16_t rawY = (int16_t)(yH << 8 | yL);
//         int16_t rawZ = (int16_t)(zH << 8 | zL);

//         sumX += rawX;
//         sumY += rawY;
//         sumZ += rawZ;
//       }
//       vTaskDelay(pdMS_TO_TICKS(10)); // 避免過快讀取，可以根據實際情況調整
//     }
//     *x = sumX / samples;
//     *y = sumY / samples;
//     *z = sumZ / samples;
//   }

// private:
//   void writeRegister(uint8_t reg, uint8_t val)
//   {
//     Wire.beginTransmission(address);
//     Wire.write(reg);
//     Wire.write(val);
//     Wire.endTransmission();
//   }

//   void get_gauss()
//   {
//     float lsb_by_gauss = 0;

//     switch (rng)
//     {
//     case QMC5883_RNG_30G:
//       Serial.println("QMC5883_RNG_30G");
//       lsb_by_gauss = 1000.0;
//       break;
//     case QMC5883_RNG_12G:
//       Serial.println("QMC5883_RNG_12G");
//       lsb_by_gauss = 2500.0;
//       break;
//     case QMC5883_RNG_8G:
//       Serial.println("QMC5883_RNG_8G");
//       lsb_by_gauss = 3750.0;
//       break;
//     case QMC5883_RNG_2G:
//       Serial.println("QMC5883_RNG_2G");
//       lsb_by_gauss = 15000.0;
//     }

//     gaussX = x / lsb_by_gauss;
//     gaussY = y / lsb_by_gauss;
//     gaussZ = z / lsb_by_gauss;
//   }

//   void get_raw(int16_t *rawX, int16_t *rawY, int16_t *rawZ)
//   {
//     Wire.beginTransmission(address);
//     Wire.write(0x01);
//     Wire.endTransmission();
//     Wire.requestFrom(address, 6);

//     if (Wire.available() == 6)
//     {
//       uint8_t xL = Wire.read();
//       uint8_t xH = Wire.read();
//       uint8_t yL = Wire.read();
//       uint8_t yH = Wire.read();
//       uint8_t zL = Wire.read();
//       uint8_t zH = Wire.read();

//       *rawX = (int16_t)(xH << 8 | xL);
//       *rawY = (int16_t)(yH << 8 | yL);
//       *rawZ = (int16_t)(zH << 8 | zL);
//     }
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// };
#endif