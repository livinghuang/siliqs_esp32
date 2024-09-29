#include "bsp.h"
#ifdef USE_LORAWAN
#ifndef LORAWAN_COMMUNICATION_H
#define LORAWAN_COMMUNICATION_H
#include "LoRaWan_APP.h"
#include "siliqs_heltec_esp32.h"

// 全域變數定義
extern uint8_t devEui[];
extern uint8_t appEui[];
extern uint8_t appKey[];

extern uint8_t nwkSKey[];
extern uint8_t appSKey[];
extern uint32_t devAddr;

extern uint16_t userChannelsMask[];
extern LoRaMacRegion_t loraWanRegion;
extern DeviceClass_t loraWanClass;
extern uint32_t appTxDutyCycle;
extern bool overTheAirActivation;
extern bool loraWanAdr;
extern bool isTxConfirmed;
extern uint8_t appPort;
extern uint8_t confirmedNbTrials;

class SQ_LoRaWanClass : public LoRaWanClass
{
public:
  // Constructor
  SQ_LoRaWanClass();
  void SQ_LoRaWan_Init(DeviceClass_t lorawanClass, LoRaMacRegion_t region);
  void SQ_LoRaWan_SendData(uint8_t *data, uint8_t size);
  void SQ_LoRaWan_send(void);
  void SQ_LoRaWan_Cycle(uint32_t dutyCycle);
  void SQ_LoRaWan_Sleep(DeviceClass_t classMode);
  void SQ_LoRaWan_generateDeveuiByChipID(bool simple = true);

protected:
};

#endif // LORAWAN_COMMUNICATION_H
#endif // USE_LORAWAN