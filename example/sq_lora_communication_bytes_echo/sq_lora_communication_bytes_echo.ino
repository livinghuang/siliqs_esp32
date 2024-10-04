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
  char msg[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  loraComm.send(msg, sizeof(msg));
}

void loop()
{
  char buf[6]; // Increase buffer size to accommodate larger messages
  int len = loraComm.receive(buf, sizeof(buf), 1000);
  if (len > 0)
  {
    buf[len] = '\0'; // Ensure the received string is null-terminated

    Serial.print("Received: ");
    for (int i = 0; i < len; i++)
    {
      Serial.print(buf[i]++, HEX);
      Serial.print(" ");
    }
    Serial.println();
    delay(500);
    // send echo
    loraComm.send(buf, len);
  }
  else
  {
    Serial.println("No data received.");
  }
}