#ifdef USE_LORAWAN
#ifndef LORAWAN_COMMUNICATION_H
#define LORAWAN_COMMUNICATION_H
#include "siliqs_heltec_esp32.h"
#include "LoRaWan_APP.h"

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

void prepareTxFrame(uint8_t port);
void lorawanSetup();
void lorawanLoop();

#endif // LORAWAN_COMMUNICATION_H
#endif // USE_LORAWAN