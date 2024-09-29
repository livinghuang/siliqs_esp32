#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "communication/lorawan_communication.h"

/**
 * @brief setup 函数，用于初始化系统
 *
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。
 */

void prepareTxFrame(uint8_t port)
{
  appDataSize = 4;
  memset(appData, 0, sizeof(appData));
  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;
}

/* Instance of the derived class */
SQ_LoRaWanClass SQ_LoRaWAN;
void setup()
{
  Serial.begin(115200);
  siliqs_heltec_esp32_setup();

  /* Initialize the LoRaWAN stack using the derived class */
  deviceState = DEVICE_STATE_INIT;
}

void loop()
{
  switch (deviceState)
  {
  case DEVICE_STATE_INIT:
  {
    SQ_LoRaWAN.init(loraWanClass, loraWanRegion);
    deviceState = DEVICE_STATE_JOIN;
    break;
  }
  case DEVICE_STATE_JOIN:
  {
    SQ_LoRaWAN.join(); // Call the derived class method to join
    deviceState = DEVICE_STATE_SEND;
    break;
  }
  case DEVICE_STATE_SEND:
  {
    prepareTxFrame(appPort); // Prepare data to send
    SQ_LoRaWAN.send();       // Call the derived class method to send
    deviceState = DEVICE_STATE_CYCLE;
    break;
  }
  case DEVICE_STATE_CYCLE:
  {
    // Schedule next packet transmission
    txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
    SQ_LoRaWAN.cycle(txDutyCycleTime); // Call the derived class method for cycling
    deviceState = DEVICE_STATE_SLEEP;
    break;
  }
  case DEVICE_STATE_SLEEP:
  {
    SQ_LoRaWAN.sleep(loraWanClass); // Put the device to sleep
    break;
  }
  default:
  {
    deviceState = DEVICE_STATE_INIT;
    break;
  }
  }
}
