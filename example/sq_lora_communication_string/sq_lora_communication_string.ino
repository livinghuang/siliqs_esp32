#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/lora_communication.h"

LoraCommunication loraComm;
uint8_t count = 0; // Track the count for the message

/**
 * @brief Setup function to initialize the system
 *
 * This function initializes the ESP32 board and the LoRa device,
 * and sends the initial data packet with a counter.
 */
void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO); // Initialize the ESP32 board
  loraComm.begin();                   // Initialize LoRa communication
  String msg = "Hello world: " + String(count);
  loraComm.println(msg);
}

void loop()
{

  String receivedMessage = loraComm.readString();

  if (receivedMessage.length() > 0)
  {
    Serial.print("Received: ");
    Serial.println(receivedMessage);

    // Find the number in the message
    int index = receivedMessage.lastIndexOf(": "); // Find where the number starts
    if (index != -1)
    {
      // Extract the number from the message and convert to integer
      String countStr = receivedMessage.substring(index + 2); // Extract the part after ": "
      count = countStr.toInt() + 1;                           // Increment the count
      delay(1000);
      // Prepare and send the next message with the updated count
      String Message = "Hello world: " + String(count);
      loraComm.println(Message);
    }
    else
    {
      Serial.println("No valid count found in the message.");
    }
  }
  else
  {
    Serial.println("No data received.");
  }
}