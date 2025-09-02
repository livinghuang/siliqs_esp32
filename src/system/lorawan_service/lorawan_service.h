#include "bsp.h"
#pragma once
#ifdef USE_LORAWAN

#include "siliqs_esp32.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "radiolab/src/RadioLib.h"

enum LORAWAN_SLEEP_TYPE
{
  LORAWAN_SLEEP_NONE = 0,                 // no sleeping, no radio off, spi ended
  LORAWAN_SLEEP_IN_RADIO_OFF = 1,         // use radio off
  LORAWAN_SLEEP_IN_DEEP = 2,              // use radio off and system deep sleep
  LORAWAN_SLEEP_IN_DEEP_WITH_TTN_LAW = 3, // use system deep sleep calculate minimum duty cycle delay (per FUP & law!)
};
typedef struct lorawan_params_settings
{
  int DIO1;                       // DIO1
  int BUSY;                       // BUSY
  int NRST;                       // reset
  int MISO;                       // MISO
  int MOSI;                       // MOSI
  int SCK;                        // SCK
  int NSS;                        // NSS
  uint32_t uplinkIntervalSeconds; // Unit: second, upline interval time
  bool ADR;                       // use ADR or not
  uint8_t DR;                     // Data Rate when start, if ADR is true, this will be tuned automatically
  uint16_t DutyCycleFactor;       // Duty Cycle = 1 / (DutyCycleFactor) , if 0, disable. In EU law, Duty Cycle should under 1%
  uint16_t DwellTime;             // Unit: ms, Dwell Time to limit signal airtime in single channel, In US/AU law,Dwell Time under 400ms
  bool OTAA;                      // OTAA or ABP
  bool LORAWAN_1_1;               // LORAWAN 1.1 or 1.0
  uint64_t JOINEUI;               // JOINEUI
  uint64_t DEVEUI;                // DEVEUI, if OTAA, DEVEUI will used. if ABP, DEVEUI will be ignored
  uint8_t APPxKEY[16];            // if OTAA, APPxKEY = APPKEY, if ABP, APPxKEY = APPSKEY
  uint8_t NWKxKEY[16];            // if OTAA, NWKxKEY in lorawan v1.0.x, no use, if ABP, APPxKEY = NWKSKEY
  uint32_t DEVADDR;               // DEVADDR
  uint8_t FNWKSINT[16];           // FNWKSINT, if lorawan v1.0.x, no use
  uint8_t SNWKSINT[16];           // SNWKSINT, if lorawan v1.0.x, no use
} lorawan_params_settings;

class LoRaWanService
{
public:
  // Constructor and destructor
  LoRaWanService(lorawan_params_settings *params);
  ~LoRaWanService();
  bool begin(bool autogen = true);
  void stop();
  void softSleep();
  void sleep(enum LORAWAN_SLEEP_TYPE sleep_type = LORAWAN_SLEEP_IN_DEEP_WITH_TTN_LAW);
  void set_battery_level(int level);
  void send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, uint8_t *fPortDown, bool isConfirmed);
  void send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, bool isConfirmed);
  void autoGenKeys();

  void end()
  {
    // stop();
  }

private:
  lorawan_params_settings *params;
  int begin_node();
  int active_node();
  void setParams();
  int16_t lwActivate();
};

void arrayDump(uint8_t *buffer, uint16_t len);
void debug(bool failed, const __FlashStringHelper *message, int state, bool halt);
String stateDecode(const int16_t result);
#endif // USE_LORAWAN