#include "bsp.h"
#include "siliqs_esp32.h"
#include "communication/rs485_communication.h"
/*
If you use SQC-485I development board, the pRS485_DE pin is connected to the esp32 user pin (aka GPIO9).
When you upload the code, GPIO9 may latch by USB to UART chip (aka CH340K), you could re-plug the USB cable to fix it.
*/
#include "communication/modbus_communication.h"

ModbusCommunication modbusComm(Serial1, 4800, pRS485_RO, pRS485_DI, pRS485_DE, pVext, false);

// Callback function for processing Modbus requests
void myModbusProcessor(const modbus_data_t *request, modbus_data_t *&response)
{
  // Predefined commands and corresponding responses
  uint8_t modbus_command1[] = {0x03, 0x04, 0x00, 0x09};
  uint8_t modbusResponse1[] = {
      0x12, 0x00, 0x00, 0x00, 0x31, 0x96, 0x00, 0x00, 0x05, 0x00,
      0x00, 0x94, 0x00, 0x00, 0x00, 0x01, 0x34, 0x0C, 0x03};

  uint8_t modbus_command2[] = {0x05, 0x04, 0x00, 0x14};
  uint8_t modbusResponse2[] = {
      0x28, 0x00, 0x00, 0x00, 0x31, 0x96, 0x67, 0x00, 0x05, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x00, 0x04, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
      0x35, 0x0C, 0x03, 0x00, 0x00, 0x52, 0x35, 0x10, 0x20, 0x11, 0x47};

  uint8_t modbus_command3[] = {0x02, 0x00, 0x00};
  uint8_t modbusResponse3[] = {
      0x28, 0x00, 0x00, 0x00, 0x31, 0x96, 0x67, 0x00, 0x05, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x05, 0x00, 0x00, 0x00,
      0x00, 0x04, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
  Serial.println("Processing Modbus request...");

  // Check if the request matches a predefined command
  if (request->address == 0x10 && request->function == 0x21)
  {
    Serial.println("Match found!");
    if (memcmp(request->data, modbus_command1, sizeof(modbus_command1)) == 0)
    {
      // Generate response for command 1
      Serial.println("Generate response for command 1");
      response = new modbus_data_t;
      response->address = request->address;
      response->function = request->function;
      response->length = sizeof(modbusResponse1);
      memcpy(response->data, modbusResponse1, response->length);
      return;
    }

    if (memcmp(request->data, modbus_command2, sizeof(modbus_command2)) == 0)
    {
      // Generate response for command 2
      Serial.println("Generate response for command 2");
      response = new modbus_data_t;
      response->address = request->address;
      response->function = request->function;
      response->length = sizeof(modbusResponse2);
      memcpy(response->data, modbusResponse2, response->length);
      return;
    }

    if (memcmp(request->data, modbus_command3, sizeof(modbus_command3)) == 0)
    {
      // Generate response for command 3
      Serial.println("Generate response for command 3");
      response = new modbus_data_t;
      response->address = request->address;
      response->function = request->function;
      response->length = sizeof(modbusResponse3);
      memcpy(response->data, modbusResponse3, response->length);
      return;
    }
    Serial.println("No match command found.");
  }

  Serial.println("No match found.");
  // Print the request details
  Serial.print("Received Modbus request: ");
  Serial.print("Address: ");
  Serial.print(request->address, HEX);
  Serial.print(", Function: ");
  Serial.print(request->function, HEX);
  Serial.print(", Length: ");
  Serial.print(request->length, HEX);
  Serial.print(", Data: ");
  print_bytes((uint8_t *)request->data, request->length);

  // No match found, no response needed
  response = nullptr;
}

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。
 */
void setup()
{
  siliqs_esp32_setup(); // SQ_DEBUG
  // Initialize the Modbus communication
  modbusComm.begin();

  // Start the Modbus slave task with the custom request processor
  if (modbusComm.startSlaveTask(myModbusProcessor))
  {
    Serial.println("Modbus slave task started successfully!");
  }
  else
  {
    Serial.println("Failed to start Modbus slave task.");
  }
}

void loop()
{
}