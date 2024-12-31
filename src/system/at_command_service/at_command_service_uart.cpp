#include "bsp.h"
#ifdef USE_AT_COMMAND_SERVICE

#include "at_command_service.h"
// 构造函数，传入 HardwareSerial 对象
UARTATCommandService::UARTATCommandService()
{
}

// 启动 AT 命令处理任务
void UARTATCommandService::startTask(int taskPriority, int stackSize)
{
  if (taskHandle == NULL)
  {
    xTaskCreate(taskFunction, "UARTATCommandTask", stackSize, this, taskPriority, &taskHandle);
  }
}

// 停止 AT 命令处理任务
void UARTATCommandService::stopTask()
{
  if (taskHandle != NULL)
  {
    vTaskDelete(taskHandle);
    taskHandle = NULL;
  }
}

// FreeRTOS 任务函数，用于处理 UART 数据
void UARTATCommandService::taskFunction(void *pvParameters)
{
  // 将传入的 pvParameters 转换为 UARTATCommandService 对象指针
  UARTATCommandService *service = static_cast<UARTATCommandService *>(pvParameters);
  while (true)
  {
    Serial.setTimeout(20);
    // 检查 UART 中是否有可读取的数据
    if (Serial.available())
    {
      // 读取 UART 接收到的数据
      String data = Serial.readString();
      // 使用类实例调用非静态成员函数处理数据
      service->handleIncomingData(data);
    }

    // 延迟 10 毫秒，避免过度占用 CPU
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// 实现 sendResponse 函数，通过 UART 发送响应
void UARTATCommandService::sendResponse(const String &response)
{
  Serial.print("[CMD>] ");
  Serial.println(response);
  Serial.flush();
}

// 实现 sendEchoCommand 函数，通过 UART 回显命令
void UARTATCommandService::sendEchoCommand(const String &response)
{
  if (echoEnabled)
  {
    Serial.print("[CMD<] ");
    Serial.println(response);
    Serial.flush();
  }
}
#endif