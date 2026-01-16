#include "bsp.h"
#pragma once
#ifdef USE_LORAWAN

#include "siliqs_esp32.h"
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

#define LORAWAN_DOWN_MAXLEN 255
#define LORAWAN_DOWN_QSIZE 16 // 你要幾筆緩衝就設多大

typedef struct
{
  uint32_t ts_ms;                    // 收到時間
  uint8_t fport;                     // 來源 FPort
  size_t len;                        // 資料長度
  uint8_t data[LORAWAN_DOWN_MAXLEN]; // 資料本體（拷貝進來）
} LoraDownlinkMsg_t;

#define LORAWAN_UP_MAXLEN 255
#define LORAWAN_UP_QSIZE 16 // 你要幾筆緩衝就設多大

typedef struct
{
  uint32_t ts_ms;                  // 收到時間
  uint8_t fport;                   // FPort
  size_t len;                      // 資料長度
  uint8_t data[LORAWAN_UP_MAXLEN]; // 資料本體（拷貝進來）
  uint8_t isConfirmed;
  uint8_t ack;
  uint8_t battery_level;
} LoraUplinkMsg_t;

class LoRaWanService
{
public:
  // Constructor and destructor
  LoRaWanService(lorawan_params_settings *params);
  ~LoRaWanService();
  bool begin(bool autogen = true);
  void start();
  void stop();
  void softSleep();
  void sleep(enum LORAWAN_SLEEP_TYPE sleep_type = LORAWAN_SLEEP_IN_DEEP_WITH_TTN_LAW);
  void set_battery_level(int level);
  void send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, uint8_t *fPortDown, bool isConfirmed);
  void send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, bool isConfirmed);
  void send_and_receive(bool *success, const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, uint8_t *fPortDown, bool isConfirmed);
  void send_and_receive(bool *success, const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, bool isConfirmed);
  void setCSMA(bool csmaEnabled, uint8_t maxChanges, uint8_t backoffMax, uint8_t difsSlots);
  void setClass(LoRaWANClass cls);
  void startClassC();                                                     // 進入常駐 Rx2
  void stopClassC();                                                      // 暫停（例如 TX 前）
  bool pollClassC(uint8_t *dataDown, size_t *lenDown, uint8_t *portDown); // 在主循環呼叫：若 ISR 置位，取包並解析
  void autoGenKeys();
  void setParams(lorawan_params_settings *params);
  void printParams();

  void init_downlink_queue();
  bool is_downlink_queue_empty() const;
  size_t downlink_queue_size() const;
  bool pop_from_downlink_queue(LoraDownlinkMsg_t *out);
  bool push_to_downlink_queue(const uint8_t *buf, size_t len, uint8_t fport);

  void init_uplink_queue();
  bool is_uplink_queue_empty() const;
  size_t uplink_queue_size() const;
  bool pop_from_uplink_queue(LoraUplinkMsg_t *out);
  bool push_to_uplink_queue(const uint8_t *buf, size_t len, uint8_t fport);
  uint32_t uplink_interval_ms() { return params->uplinkIntervalSeconds * 1000; }
  void end()
  {
    // stop();
  }

private:
  lorawan_params_settings *params;
  bool success_downlink_received = false;
  int begin_node();
  int active_node();
  int16_t lwActivate();
  TaskHandle_t taskHandle = nullptr;
  static QueueHandle_t s_lorawanDownQ;
  static QueueHandle_t s_lorawanUpQ;
  static SemaphoreHandle_t s_lorawanUpQMutex;
  static SemaphoreHandle_t s_lorawanDownQMutex;
  static void taskLoop(void *param);
};

void debug(bool failed, const __FlashStringHelper *message, int state, bool halt);
String stateDecode(const int16_t result);
#endif // USE_LORAWAN