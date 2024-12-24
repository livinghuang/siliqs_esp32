# Siliq_ESP32 Project

The **Siliq_ESP32 Project** is designed for the Siliqs-ESP32 development boards, which feature ESP32-based main chips such as the ESP32-C3 or ESP32-S3. These boards support Wi-Fi, BLE, and LoRa, and this project provides a robust framework to manage peripherals, along with examples to help users build their own projects efficiently.

---

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

---

## Getting Started

Follow these steps to set up the **Siliq_ESP32** library for use in your Arduino IDE:

### 1. Download the Project
1. Visit the [Siliq_ESP32 GitHub Repository](https://github.com/livinghuang/siliqs_esp32).
2. Click on the **Code** button and choose **Download ZIP**.
3. Extract the downloaded ZIP file to your Arduino libraries folder:
   - **Windows**: `Documents\Arduino\libraries`
   - **macOS**: `~/Documents/Arduino/libraries`
   - **Linux**: `~/Arduino/libraries`

### 2. Locate the `libraries` Folder
Ensure that the `Siliq_ESP32` folder inside the extracted content is correctly placed in the `libraries` folder of your Arduino setup.

### 3. Open Examples in Arduino IDE
1. Restart the Arduino IDE to refresh the library list.
2. Navigate to **File > Examples > Siliq_ESP32**.
3. Select an example to begin experimenting.

### 4. Connect Your Hardware
1. Plug your ESP32-based board into your computer via USB.
2. Select the appropriate board and port from **Tools > Board** and **Tools > Port** in the Arduino IDE.

### 5. Upload the Example Code
1. Open an example from **File > Examples > Siliq_ESP32**.
2. Click the **Upload** button to upload the code to your ESP32 board.

---

## 繁體中文版本

# Siliq_ESP32 專案

**Siliq_ESP32 專案** 是為了 Siliqs-ESP32 開發板設計，這些開發板採用 ESP32-C3 或 ESP32-S3 作為主晶片，支援 Wi-Fi、BLE 和 LoRa。該專案提供了一個穩定的框架來管理外設，並附帶示例，幫助用戶快速構建自己的專案。

---

## 功能特色

1. **相容 Arduino IDE**  
   專案完全支援 Arduino IDE，方便用戶開發和上傳程式碼。

2. **主要系統功能**  
   系統初始化由 `siliqs_esp32_setup()` 函數處理，簡化了初始化流程。

3. **除錯主控台支援**  
   提供內建的除錯主控台，便於開發與除錯。

4. **可自訂的 AT 指令框架**  
   包含可用於創建自訂 AT 指令的框架，用戶可依需求建立自己的指令。

5. **多介面系統訪問**  
   可通過以下方式訪問 Siliqs 系統：
   - 串口
   - BLE 連接

6. **BLE HTML 原始碼**  
   提供的 HTML 原始碼，能實現無縫的 BLE 連接。

7. **支援 LoRa 和 LoRaWAN**  
   - 整合 LoRa 和 LoRaWAN 庫。
   - 擴展 AS923_1 支援，適用於 8 通道專案。

8. **RS485 和 Modbus 支援**  
   包含 RS485 通訊和 Modbus 協議支援，適用於工業和自動化應用。

9. **全面的感測器驅動支援**  
   支援多種感測器驅動，例如：
   - **HCD1080**（濕度與溫度）
   - **DPS310**（壓力）
   - **MAX31865**（RTD 溫度）
   - **MPC5883**（磁力計）
   - **MPU6050**（慣性測量單元）
   - 及更多...

10. **GPS 位置驅動**  
    內建 GPS 位置功能支援。

11. **檔案系統支援**  
    使用 ESP32 內部快閃記憶體作為檔案系統，提高資料管理效率。

---

## 快速開始

請按照以下步驟設置 **Siliq_ESP32** 庫，以便在 Arduino IDE 中使用：

### 1. 下載專案
1. 前往 [Siliq_ESP32 GitHub Repository](https://github.com/livinghuang/siliqs_esp32)。
2. 點擊 **Code** 按鈕，選擇 **Download ZIP**。
3. 解壓縮 ZIP 檔案到 Arduino 的 `libraries` 資料夾：
   - **Windows**: `Documents\Arduino\libraries`
   - **macOS**: `~/Documents/Arduino/libraries`
   - **Linux**: `~/Arduino/libraries`

### 2. 確認 `libraries` 資料夾
確保解壓後的 `Siliq_ESP32` 資料夾正確放置在 Arduino 設定的 `libraries` 資料夾中。

### 3. 在 Arduino IDE 中開啟示例
1. 重啟 Arduino IDE 以刷新庫列表。
2. 移至 **File > Examples > Siliq_ESP32**。
3. 選擇一個示例進行測試。

### 4. 連接硬體
1. 通過 USB 將 ESP32 開發板連接到電腦。
2. 在 **Tools > Board** 和 **Tools > Port** 中選擇正確的開發板和連接埠。

### 5. 上傳示例程式碼
1. 在 **File > Examples > Siliq_ESP32** 中開啟示例程式碼。
2. 點擊 **Upload** 按鈕將程式碼上傳到 ESP32 開發板。

---

透過 **Siliq_ESP32** 框架，您可以輕鬆探索其豐富功能，並快速開發自己的專案。更多資訊請參考專案文件或訪問 [GitHub Repository](https://github.com/your-repo-url)。
