#ifndef LORA_COMMUNICATION_H
#define LORA_COMMUNICATION_H

#include "siliqs_heltec_esp32.h"
#include "communication.h"
#include "LoRaWan_APP.h"
#include "Arduino.h"

// LoRa parameters configuration
#define RF_FREQUENCY 915000000  // Frequency in Hz
#define TX_OUTPUT_POWER 5       // Output power in dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR 7 // Spreading factor [SF7..SF12]
#define LORA_CODINGRATE 1       // Coding rate [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Preamble length
#define LORA_SYMBOL_TIMEOUT 0   // Symbol timeout
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE 30 // Buffer size for data packets

class LoraCommunication : public Communication
{
public:
  // Constructor
  LoraCommunication(int powerPin = -1);
  // Override methods from Communication class
  void begin() override;                                                    // Initialize LoRa device
  void send(const char *data, int length) override;                         // Send data packet
  size_t receive(char *buffer, size_t length, int timeout = 1000) override; // Receive data packet
  void print(const String &data);
  void println(const String &data);
  String readString(size_t maxLength = 51);

  void setReceiveTimeout(int timeout);
};

#endif // LORA_COMMUNICATION_H