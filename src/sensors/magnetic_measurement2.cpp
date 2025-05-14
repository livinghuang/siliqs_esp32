// #include "bsp.h"
// #ifdef USE_MAGNETOMETER2
// #include <Arduino.h>
// #include "magnetic_measurement2.h"
// // QMC5883 qmc5883;
// // // #define pMQC5883_I2C_SCL 19
// // // #define pMQC5883_I2C_SDA 18
// // void magnetic_init()
// // {
// //   Serial.begin(115200);
// //   while (!Serial)
// //     ; // Wait for Serial Monitor to open (for boards like Leonardo)

// //   Serial.println("I2C Scanner Starting...");
// // }

// // void test_magnetic()
// // {
// //   Wire.begin(pMQC5883_I2C_SDA, pMQC5883_I2C_SCL); // Join I2C bus as master
// //   qmc5883.begin();
// //   Serial.println("Magnetic test");
// //   while (1)
// //   {
// //     qmc5883.fetch_data();
// //     qmc5883.print_data();
// //     delay(500);
// //     qmc5883.print_LSB();
// //     delay(500);
// //   }
// // }
// #endif