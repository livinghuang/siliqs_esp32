#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/lora_communication.h"

lora_settings params = {
    .DIO1 = 3,
    .BUSY = 4,
    .NRST = 5,
    .MISO = 6,
    .MOSI = 7,
    .SCK = 10,
    .NSS = 8,
    .FREQUENCY = 922.0,
    .BANDWIDTH = 125.0,
    .SF = 7,
    .CR = 5,
    .SYNC_WORD = 0x34,
    .OUTPUT_POWER = 22,
    .PREAMBLE_LENGTH = 8};

LoraCommunication loraComm(&params,-1);
// LoraCommunication loraComm;

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
  // loraComm.send(msg, sizeof(msg));
}

void loop()
{
  // char buf[6]; // Increase buffer size to accommodate larger messages
  // int len = loraComm.receive(buf, sizeof(buf), 1000);
  // if (len > 0)
  // {
  //   buf[len] = '\0'; // Ensure the received string is null-terminated

  //   Serial.print("Received: ");
  //   for (int i = 0; i < len; i++)
  //   {
  //     Serial.print(buf[i]++, HEX);
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  //   delay(500);
  //   // send echo
  //   loraComm.send(buf, len);
  // }
  // else
  // {
  //   Serial.println("No data received.");
  // }
}