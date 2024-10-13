#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE
#ifdef USE_NIMBLE
#include "at_command_service.h"
#include "siliqs_heltec_esp32.h"

BLEATCommandService::BLEATCommandService()
{
}

void BLEATCommandService::begin()
{
}

// 启动 AT 命令处理任务
void BLEATCommandService::startTask(int taskPriority, int stackSize)
{
  if (taskHandle == NULL)
  {
    xTaskCreate(taskFunction, "BLEATCommandTask", stackSize, this, taskPriority, &taskHandle);
  }
}

// 停止 AT 命令处理任务
void BLEATCommandService::stopTask()
{
  if (taskHandle != NULL)
  {
    vTaskDelete(taskHandle);
    taskHandle = NULL;
  }
}

// FreeRTOS 任务函数，用于处理 UART 数据
void BLEATCommandService::taskFunction(void *pvParameters)
{
  BLEATCommandService *service = static_cast<BLEATCommandService *>(pvParameters);

  while (true)
  {
    if (nimbleService.deviceConnected)
    {
      String data = nimbleService.getReceivedData();

      if (data.length() > 0)
      {
        nimbleService.setReceivedData(""); // Clear buffer after processing
        console.log(sqINFO, "[BLE] Received data: " + data);
        data = data + "\r\n";
        service->handleIncomingData(data); // Process the AT command
      }
    }
    // Delay 10 ms to avoid CPU overload
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
// 实现 sendResponse 函数，通过 BLE 发送响应
void BLEATCommandService::sendResponse(const String &response)
{
  String response_Data = ">" + response;
  console.log(sqINFO, response_Data);
  nimbleService.sendData(response_Data.c_str());
}

// 实现 sendEchoCommand 函数，通过 BLE 回显命令
void BLEATCommandService::sendEchoCommand(const String &response)
{
  if (echoEnabled)
  {
    String response_Data = "<" + response;
    console.log(sqINFO, response_Data);
    nimbleService.sendData(response_Data.c_str());
  }
}
#endif
#endif