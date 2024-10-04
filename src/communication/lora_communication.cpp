#include "bsp.h"
#ifdef USE_LORA
#include "lora_communication.h"

#define RF_FREQUENCY 915000000 // Hz

#define TX_OUTPUT_POWER 5 // dBm

#define LORA_BANDWIDTH 0        // [0: 125 kHz,
                                //  1: 250 kHz,
                                //  2: 500 kHz,
                                //  3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5,
                                //  2: 4/6,
                                //  3: 4/7,
                                //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char *rxpacket;
int rx_buffer_size;
double txNumber;
int Rssi, rxSize, Snr;
bool lora_idle = true;

static RadioEvents_t RadioEvents;
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxDone(void);
void OnTxTimeout(void);

// Constructor
LoraCommunication::LoraCommunication(int powerPin)
{
  this->powerPin = powerPin;
}

// Initialize LoRa device
void LoraCommunication::begin()
{
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  txNumber = 0;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  lora_idle = true;
  console.log(sqINFO, "Lora communication initialized");
}

void LoraCommunication::send(const char *data, int length)
{
  if (lora_idle == true)
  {
    console.log(sqINFO, "Lora sending data: ");
    console.log(sqINFO, data, length);
    Radio.Send((uint8_t *)data, length); // send the package out
    lora_idle = false;
    while (lora_idle == false)
    {
      Radio.IrqProcess();
    }
  }
  else
  {
    console.log(sqINFO, "Lora is busy");
  }
}

size_t LoraCommunication::receive(char *buffer, size_t length, int timeout)
{
  if (lora_idle)
  {
    lora_idle = false;
    rx_buffer_size = length;
    rxpacket = buffer;
    console.log(sqINFO, "into RX mode");
    Radio.Rx(0);
    uint32_t start = millis();
    while (millis() - start < timeout)
    {
      if (lora_idle == true)
      {
        console.log(sqINFO, "Received data:");
        console.log(sqINFO, rxpacket, rxSize);
        console.log(sqINFO, "Data size: " + String(rxSize) + " Rssi: " + String(Rssi) + " Snr: " + String(Snr));
        return rxSize;
      }
      Radio.IrqProcess();
    }
    lora_idle = true;
    console.log(sqINFO, "No data received");
  }
  else
  {
    console.log(sqINFO, "Lora is busy");
  }
  return 0;
}

void LoraCommunication::print(const String &data)
{
  send(data.c_str(), data.length());
}

void LoraCommunication::println(const String &data)
{
  send(data.c_str(), data.length());
  send("\n", 1);
}

String LoraCommunication::readString(size_t maxLength)
{
  char buffer[maxLength];
  receive(buffer, maxLength);
  return String(buffer); // Convert the buffer to a String
}

void OnTxDone(void)
{
  console.log(sqINFO, "TX done......");
  lora_idle = true;
}

void OnTxTimeout(void)
{
  Radio.Sleep();
  console.log(sqINFO, "TX Timeout......");
  lora_idle = true;
}
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  Rssi = rssi;
  rxSize = size;
  Snr = snr;
  if (rx_buffer_size > size)
  {
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
  }
  else
  {
    memcpy(rxpacket, payload, rx_buffer_size);
    rxpacket[rx_buffer_size] = '\0';
  }
  Radio.Sleep();
  lora_idle = true;
}
#endif