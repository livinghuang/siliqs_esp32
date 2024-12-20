# Siliq_ESP32 Project

The **Siliq_ESP32 Project** is designed for the Siliqs-ESP32 development boards, which feature ESP32-based main chips such as the ESP32-C3 or ESP32-S3. These boards support Wi-Fi, BLE, and LoRa, and this project provides a robust framework to manage peripherals, along with examples to help users build their own projects efficiently.

## Features

1. **Arduino IDE Compatibility**  
   The project is fully compatible with the Arduino IDE, making it easy to develop and upload code.

2. **Main System Functionality**  
   The main system setup is handled through the `siliqs_esp32_setup()` function, simplifying initialization.

3. **Debug Console Support**  
   Built-in support for debug console printing to help with development and debugging.

4. **Customizable AT Command Framework**  
   Includes a framework for creating custom AT commands, allowing users to build commands tailored to their needs.

5. **Multi-Interface System Access**  
   Access the Siliqs system via:
   - Serial port
   - BLE connection

6. **BLE HTML Source Code**  
   Provided HTML source code enables seamless BLE connections.

7. **LoRa and LoRaWAN Support**  
   - Integrated libraries for LoRa and LoRaWAN.
   - Extended AS923_1 support for 8-channel operation in projects.

8. **RS485 and Modbus Library Support**  
   Includes support for RS485 communication and Modbus protocol, enabling robust industrial and automation applications.

9. **Comprehensive Sensor Drivers**  
   Includes drivers for various sensors:
   - **HCD1080** (Humidity and Temperature)
   - **DPS310** (Pressure)
   - **MAX31865** (RTD Temperature)
   - **MPC5883** (Magnetometer)
   - **MPU6050** (IMU)
   - And more...

10. **GPS Location Driver**  
    Built-in support for GPS location functionality.

11. **File System Support**  
    Enables the use of ESP32's internal flash as a file system for efficient data management.

## Get Started

To begin using the Siliq_ESP32 framework, follow the provided examples and documentation to set up your project and explore the rich set of features it offers.
