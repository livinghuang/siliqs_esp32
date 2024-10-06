# SQ BLE Service for ESP32

This is a BLE (Bluetooth Low Energy) service built for the ESP32 microcontroller using the Arduino framework. It provides a simple way to handle BLE server functionality, including sending and receiving data over UART-like characteristics, scanning for nearby BLE devices, and managing client connections.

## Features

	•	BLE Server Initialization: Set up a BLE server with customizable device names.
	•	Data Transmission: Send data to clients via notifications using a UART-like characteristic.
	•	Data Reception: Receive data from connected clients and store it for later processing.
	•	BLE Device Scanning: Scan for nearby BLE devices, with the ability to update discovered devices.
	•	Client Connection Management: Automatically handles client connections and reconnections, including advertising restart after disconnects.

## Installation

### Prerequisites

	•	ESP32 Development Board: Ensure you have an ESP32 board compatible with the Arduino IDE.
	•	Arduino IDE: Set up the Arduino IDE with ESP32 board support.
	•	BLE Library: The code uses the ESP32 BLE library for BLE functionality.

### Adding to Your Project

To include this BLE service in your project, place the ble_service.h and ble_service.cpp files into your project directory. Also, make sure to enable BLE functionality by defining USE_BLE in your project.

## Usage

1. Initialization

To initialize the BLE service, call the init() method with a timeout and optional device name prefix.

``` cpp
SQ_BLEService.init(30000, "MyBLEDevice-");
```

This will create a BLE device with a name like MyBLEDevice-1234 where 1234 is derived from the ESP32’s MAC address.

2. Sending Data

The BLE service allows you to send data to connected clients via the sendData() methods. You can either pass a String or a const char*:

``` cpp
// Send a string
SQ_BLEService.sendData("Hello BLE Client!");

// Send a const char*
const char *data = "Data from ESP32";
SQ_BLEService.sendData(data);
```

3. Receiving Data

When data is received from a client, it is automatically stored using the setReceivedData() method, which can be accessed using getReceivedData():

``` cpp
String received = SQ_BLEService.getReceivedData();
Serial.println("Received from BLE Client: " + received);
```

4. Scanning for BLE Devices

To scan for nearby BLE devices, you can call the scanDevices() method and specify the scan duration in seconds:

``` cpp
SQ_BLEService.scanDevices(5); // Scan for 5 seconds
```

Discovered devices are stored in the discoveredDevices vector. If a device already exists in the list, the old entry is removed, and the new one is added with updated data (e.g., RSSI).

5. FreeRTOS Task Management

The BLE service runs in the background using FreeRTOS tasks. You can call bleTaskWrapper() to handle task management:

``` cpp
xTaskCreate(SQ_BLEServiceClass::bleTaskWrapper, "BLE Task", 4096, NULL, 1, NULL);
```

6. Stopping the Service

To stop the BLE service and free resources, call the stop() method:

``` cpp
SQ_BLEService.stop();
```

This will stop advertising, disconnect any clients, and clean up the BLE stack.

## API Overview

### SQ_BLEServiceClass Methods

	•	void init(unsigned long timeout, String namePrefix = "BLE-"): Initializes the BLE server and sets up advertising.
	•	void stop(): Stops the BLE service and deinitializes BLE stack.
	•	void sendData(const char *data): Sends data to connected clients via notifications.
	•	void sendData(String data): Sends data using a String object.
	•	void setReceivedData(const String &data): Stores received data.
	•	String getReceivedData(): Retrieves the last received data.
	•	void scanDevices(int scanTime): Scans for nearby BLE devices and updates the discovered devices list.
	•	void task(void *pvParameters): FreeRTOS task function to manage BLE operations.
	•	static void bleTaskWrapper(void *pvParameters): Task wrapper for FreeRTOS task creation.

### Data Structures

	•	std::vector<BLEAdvertisedDevice> discoveredDevices: Stores discovered BLE devices during scans. Devices are replaced if re-discovered with updated information (e.g., RSSI).

#### Example

``` cpp
#include "ble_service.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize BLE service
  SQ_BLEService.init(30000, "MyDevice-");

  // Send data periodically
  SQ_BLEService.sendData("Hello from ESP32!");

  // Scan for devices
  SQ_BLEService.scanDevices(5);  // Scanning for 5 seconds
}

void loop() {
  // Check received data
  if (SQ_BLEService.getReceivedData().length() > 0) {
    Serial.println("Data received from client: " + SQ_BLEService.getReceivedData());
  }

  delay(1000);
}
```

## License

This project is licensed under the MIT License.

# SQ BLE 服務 for ESP32

這是一個為 ESP32 微控制器構建的 BLE（低功耗藍牙）服務，基於 Arduino 框架。它提供了一個簡單的方式來處理 BLE 伺服器功能，包括透過類似 UART 的特性進行數據傳輸與接收、掃描附近的 BLE 設備以及管理客戶端連接。

## 功能特點

	•	BLE 伺服器初始化：設置具有自定義設備名稱的 BLE 伺服器。
	•	數據傳輸：透過通知使用 UART 類型的特性向客戶端發送數據。
	•	數據接收：從已連接的客戶端接收數據並存儲以供後續處理。
	•	BLE 設備掃描：掃描附近的 BLE 設備，並能夠更新已發現的設備。
	•	客戶端連接管理：自動處理客戶端連接與重新連接，包括在斷開連接後重新開始廣播。

## 安裝

### 先決條件

	•	ESP32 開發板：確保您使用的 ESP32 開發板支持 Arduino IDE。
	•	Arduino IDE：設置支持 ESP32 開發板的 Arduino IDE。
	•	BLE 函式庫：本代碼使用 ESP32 BLE 函式庫 提供 BLE 功能。

### 將程式碼加入您的專案

要將這個 BLE 服務納入您的專案，將 ble_service.h 和 ble_service.cpp 文件放入您的項目目錄中。確保在專案中定義 USE_BLE 以啟用 BLE 功能。

## 使用方式

1. 初始化

要初始化 BLE 服務，請使用 init() 方法，傳入超時時間和可選的設備名稱前綴。

``` cpp
SQ_BLEService.init(30000, "MyBLEDevice-");
```

這將創建一個名稱類似於 MyBLEDevice-1234 的 BLE 設備，其中 1234 是來自 ESP32 MAC 地址的後四位。

2. 發送數據

BLE 服務允許您透過 sendData() 方法向已連接的客戶端發送數據。可以傳遞 String 或 const char*：

``` cpp
// 發送字串
SQ_BLEService.sendData("Hello BLE Client!");

// 發送 const char*
const char *data = "Data from ESP32";
SQ_BLEService.sendData(data);
```

3. 接收數據

當從客戶端接收到數據時，會自動使用 setReceivedData() 方法將其存儲，並可以使用 getReceivedData() 進行訪問：

``` cpp
String received = SQ_BLEService.getReceivedData();
Serial.println("Received from BLE Client: " + received);
```

4. 掃描 BLE 設備

要掃描附近的 BLE 設備，可以調用 scanDevices() 方法並指定掃描時間（秒為單位）：

``` cpp
SQ_BLEService.scanDevices(5); // 掃描 5 秒
```

發現的設備會存儲在 discoveredDevices 向量中。如果設備已存在列表中，舊的條目會被刪除，並添加新的條目來更新數據（如 RSSI）。

5. FreeRTOS 任務管理

BLE 服務在後台運行使用 FreeRTOS 任務。您可以調用 bleTaskWrapper() 來處理任務管理：

``` cpp
xTaskCreate(SQ_BLEServiceClass::bleTaskWrapper, "BLE Task", 4096, NULL, 1, NULL);
```

6. 停止服務

要停止 BLE 服務並釋放資源，請調用 stop() 方法：

``` cpp
SQ_BLEService.stop();
```

這將停止廣播，斷開任何客戶端並清理 BLE 堆疊。

## API 概述

### SQ_BLEServiceClass 方法

	•	void init(unsigned long timeout, String namePrefix = "BLE-")：初始化 BLE 伺服器並設置廣播。
	•	void stop()：停止 BLE 服務並取消初始化 BLE 堆疊。
	•	void sendData(const char *data)：透過通知向已連接的客戶端發送數據。
	•	void sendData(String data)：使用 String 對象發送數據。
	•	void setReceivedData(const String &data)：存儲接收到的數據。
	•	String getReceivedData()：檢索最後接收到的數據。
	•	void scanDevices(int scanTime)：掃描附近的 BLE 設備並更新發現的設備列表。
	•	void task(void *pvParameters)：FreeRTOS 任務函數，管理 BLE 操作。
	•	static void bleTaskWrapper(void *pvParameters)：FreeRTOS 任務創建的包裝函數。

### 數據結構

	•	std::vector<BLEAdvertisedDevice> discoveredDevices：在掃描過程中存儲發現的 BLE 設備。當設備被重新發現時，會用新的信息（如 RSSI）替換舊的條目。

### 範例

``` cpp
#include "ble_service.h"

void setup() {
  Serial.begin(115200);
  
  // 初始化 BLE 服務
  SQ_BLEService.init(30000, "MyDevice-");

  // 定期發送數據
  SQ_BLEService.sendData("Hello from ESP32!");

  // 掃描設備
  SQ_BLEService.scanDevices(5);  // 掃描 5 秒
}

void loop() {
  // 檢查接收到的數據
  if (SQ_BLEService.getReceivedData().length() > 0) {
    Serial.println("Data received from client: " + SQ_BLEService.getReceivedData());
  }

  delay(1000);
}
```

## 授權

此項目基於 MIT 許可證開放。