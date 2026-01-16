#include "bsp.h"
#ifdef USE_LORAWAN // Only compile when USE_LORAWAN is enabled
#include "lorawan_service.h"
#include "radiolab/src/RadioLib.h"
#include <Preferences.h>
Preferences store;
RTC_DATA_ATTR uint16_t bootCountSinceUnsuccessfulJoin = 0;
RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];

QueueHandle_t LoRaWanService::s_lorawanDownQ = nullptr;
QueueHandle_t LoRaWanService::s_lorawanUpQ = nullptr;

SemaphoreHandle_t LoRaWanService::s_lorawanUpQMutex = nullptr;
SemaphoreHandle_t LoRaWanService::s_lorawanDownQMutex = nullptr;

void LoRaWanService::init_uplink_queue()
{
  console.log(sqINFO, "[LoRaWAN] init uplink queue");
  if (!s_lorawanUpQ)
  {
    s_lorawanUpQ = xQueueCreate(LORAWAN_UP_QSIZE, sizeof(LoraUplinkMsg_t));
    configASSERT(s_lorawanUpQ);
  }

  if (!s_lorawanUpQMutex)
  {
    s_lorawanUpQMutex = xSemaphoreCreateMutex();
    configASSERT(s_lorawanUpQMutex);
  }
}

bool LoRaWanService::is_uplink_queue_empty() const
{
  if (!s_lorawanUpQ)
    return true;
  return uxQueueMessagesWaiting(s_lorawanUpQ) == 0;
}

size_t LoRaWanService::uplink_queue_size() const
{
  return s_lorawanUpQ ? uxQueueMessagesWaiting(s_lorawanUpQ) : 0;
}

bool LoRaWanService::pop_from_uplink_queue(LoraUplinkMsg_t *out)
{
  if (!s_lorawanUpQ || !s_lorawanUpQMutex || !out)
    return false;

  const TickType_t takeTimeout = pdMS_TO_TICKS(50);

  // optional: 先記一個時間點用於 debug（不用放在 critical）
  uint32_t t0 = millis();

  // 嘗試取得 mutex（不要在 mutex 內做昂貴的工作）
  if (xSemaphoreTake(s_lorawanUpQMutex, takeTimeout) != pdTRUE)
  {
    // 取得不到 mutex -> 視情況決定是否重試或直接回失敗
    // 這裡直接回失敗（呼叫端可選擇重試或記錄）
    return false;
  }

  // 臨界區：盡量把工作縮到最小 —— 只做 xQueueReceive
  bool ok = false;
  if (xQueueReceive(s_lorawanUpQ, out, 0) == pdPASS)
  {
    ok = true;
  }

  // 立即釋放 mutex（臨界區結束）
  xSemaphoreGive(s_lorawanUpQMutex);

  // 在臨界區外做 log（避免延長 mutex 持有時間）
  if (ok)
  {
    console.log(sqINFO, "[LoRaWAN] pop uplink (mutex protected) - got msg");
  }
  else
  {
    // optional: 只在 debug 或頻繁失敗時印出，避免 log 浪費時間
    console.log(sqDEBUG, "[LoRaWAN] pop uplink empty or failed" + String(millis() - t0) + "ms wait");
  }
  return ok;
}

bool LoRaWanService::push_to_uplink_queue(const uint8_t *upBuffer, size_t upLen, uint8_t upPort)
{
  // 先做基本檢查（在 memcpy 前）
  if (!s_lorawanUpQ || !s_lorawanUpQMutex || !upBuffer)
  {
    return false;
  }
  console.log(sqINFO, "[LoRaWAN] push uplink");
  // 使用區域變數（非 static）以避免 race / 共用資料問題
  LoraUplinkMsg_t msg;
  msg.ts_ms = (uint32_t)millis();
  msg.fport = upPort;
  msg.len = (upLen > LORAWAN_UP_MAXLEN) ? LORAWAN_UP_MAXLEN : (uint8_t)upLen;
  memcpy(msg.data, upBuffer, msg.len);
  // 取得 mutex（不要用永遠等待，避免死鎖；timeout 可依需求調整）
  const TickType_t takeTimeout = pdMS_TO_TICKS(50);
  if (xSemaphoreTake(s_lorawanUpQMutex, takeTimeout) != pdTRUE)
  {
    // 取得不到 mutex，視情況回報失敗或重試
    console.log(sqWARN, "[LoRaWAN] uplink mutex take failed");
    return false;
  }
  bool result = false;
  // 嘗試送入 queue（非阻塞）
  if (xQueueSend(s_lorawanUpQ, &msg, 0) == pdPASS)
  {
    result = true;
  }
  else
  {
    // queue 滿了：在 mutex 保護下 pop 最舊一筆再 push（達到覆寫效果）
    LoraUplinkMsg_t discarded;
    if (xQueueReceive(s_lorawanUpQ, &discarded, 0) == pdTRUE)
    {
      if (xQueueSend(s_lorawanUpQ, &msg, 0) == pdPASS)
      {
        result = true;
        console.log(sqINFO, "[LoRaWAN] uplink queue full, popped oldest and pushed new");
      }
      else
      {
        console.log(sqWARN, "[LoRaWAN] uplink push failed after pop");
      }
    }
    else
    {
      // 理論上不會發生（因為 queueSend 失敗表示滿），但處理保險情況
      console.log(sqERROR, "[LoRaWAN] unexpected queueReceive failed");
    }
  }
  // 釋放 mutex
  xSemaphoreGive(s_lorawanUpQMutex);
  return result;
}

void LoRaWanService::init_downlink_queue()
{
  console.log(sqINFO, "[LoRaWAN] init downlink queue");
  if (!s_lorawanDownQ)
  {
    s_lorawanDownQ = xQueueCreate(LORAWAN_DOWN_QSIZE, sizeof(LoraDownlinkMsg_t));
    configASSERT(s_lorawanDownQ);
  }
  if (!s_lorawanDownQMutex)
  {
    s_lorawanDownQMutex = xSemaphoreCreateMutex();
    configASSERT(s_lorawanDownQMutex);
  }
}

bool LoRaWanService::is_downlink_queue_empty() const
{
  if (!s_lorawanDownQ || !s_lorawanDownQMutex)
    return true;

  // 短 timeout，避免長時間阻塞
  const TickType_t takeTimeout = pdMS_TO_TICKS(20);
  if (xSemaphoreTake(s_lorawanDownQMutex, takeTimeout) != pdTRUE)
  {
    // 取得不到 mutex 時採保守回傳（視情況可改為重試）
    return true;
  }

  UBaseType_t cnt = uxQueueMessagesWaiting(s_lorawanDownQ);
  xSemaphoreGive(s_lorawanDownQMutex);
  return cnt == 0;
}

size_t LoRaWanService::downlink_queue_size() const
{
  if (!s_lorawanDownQ || !s_lorawanDownQMutex)
    return 0;

  const TickType_t takeTimeout = pdMS_TO_TICKS(20);
  if (xSemaphoreTake(s_lorawanDownQMutex, takeTimeout) != pdTRUE)
  {
    // 取得不到 mutex 時採保守回傳 0（或可回 UINT_MAX 表示錯誤）
    return 0;
  }

  UBaseType_t cnt = uxQueueMessagesWaiting(s_lorawanDownQ);
  xSemaphoreGive(s_lorawanDownQMutex);
  return (size_t)cnt;
}

bool LoRaWanService::push_to_downlink_queue(const uint8_t *downBuffer, size_t downLen, uint8_t downPort)
{
  if (!s_lorawanDownQ || !s_lorawanDownQMutex || !downBuffer)
    return false;

  console.log(sqINFO, "[LoRaWAN] push downlink");

  // 使用區域變數，避免 static 造成的 race
  LoraDownlinkMsg_t msg;
  msg.ts_ms = millis();
  msg.fport = downPort;
  msg.len = (downLen > LORAWAN_DOWN_MAXLEN) ? LORAWAN_DOWN_MAXLEN : (uint8_t)downLen;
  memcpy(msg.data, downBuffer, msg.len);

  // 取得 mutex（不要永遠等，避免死鎖或長時間阻塞）
  const TickType_t takeTimeout = pdMS_TO_TICKS(50);
  if (xSemaphoreTake(s_lorawanDownQMutex, takeTimeout) != pdTRUE)
  {
    console.log(sqWARN, "[LoRaWAN] downlink mutex take failed");
    return false;
  }

  bool result = false;
  if (xQueueSend(s_lorawanDownQ, &msg, 0) == pdPASS)
  {
    result = true;
  }
  else
  {
    // queue 滿：pop 最舊一筆再 push（在 mutex 保護下安全）
    LoraDownlinkMsg_t discarded;
    if (xQueueReceive(s_lorawanDownQ, &discarded, 0) == pdTRUE)
    {
      if (xQueueSend(s_lorawanDownQ, &msg, 0) == pdPASS)
      {
        result = true;
        console.log(sqINFO, "[LoRaWAN] downlink queue full, popped oldest and pushed new");
      }
      else
      {
        console.log(sqWARN, "[LoRaWAN] downlink push failed after pop");
      }
    }
    else
    {
      // 理論上不會到這裡，保險處理
      console.log(sqERROR, "[LoRaWAN] downlink unexpected receive failed");
    }
  }

  xSemaphoreGive(s_lorawanDownQMutex);
  return result;
}
bool LoRaWanService::pop_from_downlink_queue(LoraDownlinkMsg_t *out)
{
  if (!s_lorawanDownQ || !s_lorawanDownQMutex || !out)
    return false;

  const TickType_t takeTimeout = pdMS_TO_TICKS(50);
  // 先嘗試取得 mutex（不要在 mutex 內做昂貴操作）
  if (xSemaphoreTake(s_lorawanDownQMutex, takeTimeout) != pdTRUE)
  {
    // 取得不到 mutex -> 視情況回傳失敗（呼叫端可決定重試）
    return false;
  }

  // 臨界區：只做必須的 xQueueReceive
  bool ok = (xQueueReceive(s_lorawanDownQ, out, 0) == pdPASS);

  // 立即釋放 mutex（縮短臨界區）
  xSemaphoreGive(s_lorawanDownQMutex);

  // 在臨界區外做 log
  if (ok)
  {
    console.log(sqINFO, "[LoRaWAN] pop downlink - got fport=%u len=%u", out->fport, out->len);
  }
  else
  {
    console.log(sqDEBUG, "[LoRaWAN] pop downlink - empty or failed");
  }

  return ok;
}

const LoRaWANBand_t Region = REGION;
const uint8_t subBand = SUB_BAND; // For US915, change this to 2, otherwise leave on 0

// SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
// Global (or wherever you create the radio)
SX1262 radio = SX1262(new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY,
                                 SPI, SPISettings(2000000, MSBFIRST, SPI_MODE0)));
RTC_DATA_ATTR LoRaWANNode node(&radio, &Region, subBand);
void print_bytes_reverse(uint8_t *data, int length);
void print_bytes(uint8_t *data, int length);
// Constructor
LoRaWanService::LoRaWanService(lorawan_params_settings *params)
{
  this->params = params;
  pinMode(LORA_BUSY, INPUT);
  pinMode(LORA_DIO1, INPUT);
  pinMode(LORA_NSS, OUTPUT);
  pinMode(LORA_NRST, OUTPUT);
  digitalWrite(LORA_NRST, LOW);
}

// Destructor
void LoRaWanService::start()
{
  init_downlink_queue();
  init_uplink_queue();
  // Start the LoRaWAN service

  xTaskCreatePinnedToCore(
      taskLoop,
      "lorawan_task",
      8192,
      this,
      1,
      &taskHandle,
      0);
}

// Destructor
LoRaWanService::~LoRaWanService()
{
  stop(); // Ensure the service is stopped and resources are released
}

void LoRaWanService::autoGenKeys()
{
  console.log(sqINFO, F("Auto generate keys"));
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("ESP32ChipID=%04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  params->DEVADDR = (uint32_t)(chipid >> 32) * (uint32_t)chipid;
  Serial.print("DEVADDR:");
  print_bytes_reverse((uint8_t *)&params->DEVADDR, sizeof(params->DEVADDR));
  char chipidStr[17];
  snprintf(chipidStr, sizeof(chipidStr), "%016llx", chipid);

  memcpy(&params->DEVEUI, &chipid, sizeof(params->DEVEUI));
  Serial.print("DEVEUI:");
  print_bytes_reverse((uint8_t *)&params->DEVEUI, sizeof(params->DEVEUI));

  memcpy(&params->APPxKEY, chipidStr, 16);
  Serial.print("APPxKEY:");
  print_bytes((uint8_t *)&params->APPxKEY, sizeof(params->APPxKEY));
  memcpy(&params->NWKxKEY, chipidStr, 16);
  Serial.print("NWKxKEY:");
  print_bytes((uint8_t *)&params->NWKxKEY, sizeof(params->NWKxKEY));
  memcpy(&params->FNWKSINT, chipidStr, 16);
  Serial.print("FNWKSINT:");
  print_bytes((uint8_t *)&params->FNWKSINT, sizeof(params->FNWKSINT));
  memcpy(&params->SNWKSINT, chipidStr, 16);
  Serial.print("SNWKSINT:");
  print_bytes((uint8_t *)&params->SNWKSINT, sizeof(params->SNWKSINT));
}
int LoRaWanService::begin_node()
{
  console.log(sqINFO, F("Begin node"));
  int16_t state = RADIOLIB_ERR_NONE;
  /*
  To activate a LoRaWAN 1.1 session, supply all the required keys:

  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  node.beginABP(devAddr, fNwkSIntKey, sNwkSIntKey, nwkSEncKey, appSKey);
  To activate a LoRaWAN 1.0.4 session, set the keys that are not available to NULL:

  node.beginOTAA(joinEUI, devEUI, NULL, appKey);
  node.beginABP(devAddr, NULL, NULL, nwkSEncKey, appSKey);
  */

  if (params->OTAA)
  {
    if (params->LORAWAN_1_1 == true)
    {
      console.log(sqINFO, "LoRaWAN 1.1 OTAA");
      state = node.beginOTAA(params->JOINEUI, params->DEVEUI, params->NWKxKEY, params->APPxKEY);
    }
    else
    {
      console.log(sqINFO, "LoRaWAN 1.0.4 OTAA");
      state = node.beginOTAA(params->JOINEUI, params->DEVEUI, NULL, params->APPxKEY);
    }
  }
  else
  {
    if (params->LORAWAN_1_1 == true)
    {
      console.log(sqINFO, "LoRaWAN 1.1 ABP");
      state = node.beginABP(params->DEVADDR, params->FNWKSINT, params->SNWKSINT, params->NWKxKEY, params->APPxKEY);
    }
    else
    {
      console.log(sqINFO, "LoRaWAN 1.0.4 ABP");
      state = node.beginABP(params->DEVADDR, NULL, NULL, params->NWKxKEY, params->APPxKEY);
    }
  }
  return state;
}

int LoRaWanService::active_node()
{
  int16_t state = RADIOLIB_ERR_NONE;
  console.log(sqINFO, F("Active node"));
  if (params->OTAA)
  {
    console.log(sqINFO, "Active in OTAA");
    state = node.activateOTAA();
  }
  else
  {
    console.log(sqINFO, "Active in ABP");
    state = node.activateABP();
  }
  return state;
}

void LoRaWanService::printParams()
{
  if (params == nullptr)
  {
    console.log(sqERROR, "LoRaWAN params not initialized!");
    return;
  }

  console.log(sqINFO, "========== LoRaWAN Parameters ==========");

  // --- 基本硬體腳位設定 ---
  console.log(sqINFO, "[Hardware Pins]");
  console.log(sqINFO, "DIO1 : %d", params->DIO1);
  console.log(sqINFO, "BUSY : %d", params->BUSY);
  console.log(sqINFO, "NRST : %d", params->NRST);
  console.log(sqINFO, "MISO : %d", params->MISO);
  console.log(sqINFO, "MOSI : %d", params->MOSI);
  console.log(sqINFO, "SCK  : %d", params->SCK);
  console.log(sqINFO, "NSS  : %d", params->NSS);

  // --- 基本通訊設定 ---
  console.log(sqINFO, "\n[Communication Settings]");
  console.log(sqINFO, "uplinkIntervalSeconds : %u s", params->uplinkIntervalSeconds);
  console.log(sqINFO, "ADR                  : %s", params->ADR ? "true" : "false");
  console.log(sqINFO, "DR                   : %u", params->DR);
  console.log(sqINFO, "DutyCycleFactor      : %u (DutyCycle = 1/%.2f%%)",
              params->DutyCycleFactor, 100.0f / params->DutyCycleFactor);
  console.log(sqINFO, "DwellTime            : %u ms", params->DwellTime);

  // --- 通訊協定版本與模式 ---
  console.log(sqINFO, "\n[LoRaWAN Mode]");
  console.log(sqINFO, "OTAA          : %s", params->OTAA ? "true" : "false");
  console.log(sqINFO, "LORAWAN_1_1   : %s", params->LORAWAN_1_1 ? "true" : "false");

  // --- 位址與金鑰 ---
  console.log(sqINFO, "\n[Identifiers]");
  console.log(sqINFO, "JOINEUI :");
  print_bytes_reverse((uint8_t *)&params->JOINEUI, sizeof(params->JOINEUI));

  console.log(sqINFO, "DEVEUI  :");
  print_bytes_reverse((uint8_t *)&params->DEVEUI, sizeof(params->DEVEUI));

  console.log(sqINFO, "DEVADDR :");
  print_bytes_reverse((uint8_t *)&params->DEVADDR, sizeof(params->DEVADDR));

  console.log(sqINFO, "\n[Keys]");
  console.log(sqINFO, "APPxKEY :");
  print_bytes(params->APPxKEY, sizeof(params->APPxKEY));

  console.log(sqINFO, "NWKxKEY :");
  print_bytes(params->NWKxKEY, sizeof(params->NWKxKEY));

  console.log(sqINFO, "FNWKSINT :");
  print_bytes(params->FNWKSINT, sizeof(params->FNWKSINT));

  console.log(sqINFO, "SNWKSINT :");
  print_bytes(params->SNWKSINT, sizeof(params->SNWKSINT));

  // --- OTAA / ABP 模式提示 ---
  console.log(sqINFO, "\n[Mode Summary]");
  if (params->OTAA)
  {
    console.log(sqINFO, "Mode : OTAA");
    if (params->LORAWAN_1_1)
      console.log(sqINFO, "LoRaWAN 1.1 OTAA -> APPxKEY=AppKey, NWKxKEY=NwkKey");
    else
      console.log(sqINFO, "LoRaWAN 1.0.x OTAA -> NWKxKEY not used");
  }
  else
  {
    console.log(sqINFO, "Mode : ABP");
    if (params->LORAWAN_1_1)
      console.log(sqINFO, "LoRaWAN 1.1 ABP -> APPxKEY=AppSKey, NWKxKEY=NwkSKey, FNWKSINT/SNWKSINT valid");
    else
      console.log(sqINFO, "LoRaWAN 1.0.x ABP -> APPxKEY=AppSKey, NWKxKEY=NwkSKey, FNWKSINT/SNWKSINT not used");
  }

  console.log(sqINFO, "========================================");
}

void LoRaWanService::setParams(lorawan_params_settings *params)
{
  int16_t state = RADIOLIB_ERR_NONE;
  printParams();
  // Enable the ADR algorithm (on by default which is preferable)
  if (params->ADR == true)
  {
    console.log(sqINFO, "Enable ADR");
  }
  else
  {
    console.log(sqINFO, "Disable ADR");
  }
  node.setADR(params->ADR);
  // Set a datarate to start off with
  node.setDatarate(params->DR);
  // / Duty Cycle = 1 / (DutyCycleFactor) , if 0, disable. In EU law, Duty Cycle should under 1%
  node.setDutyCycle(params->DutyCycleFactor ? true : false, params->DutyCycleFactor);
  // Unit: ms, Dwell Time to limit signal airtime in single channel, In US/AU law,Dwell Time under 400ms
  node.setDwellTime(params->DwellTime ? true : false, params->DwellTime);
}

int16_t LoRaWanService::lwActivate()
{
  int16_t state = RADIOLIB_ERR_UNKNOWN;

  // setup the OTAA session information
  // node.beginOTAA(params->JOINEUI, params->DEVEUI, params->NWKxKEY, params->APPxKEY);
  begin_node();

  console.log(sqINFO, F("Recalling LoRaWAN nonces & session"));
  // ##### setup the flash storage
  store.begin("radiolib");
  // ##### if we have previously saved nonces, restore them and try to restore session as well
  if (store.isKey("nonces"))
  {
    uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
    store.getBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // get them from the store
    state = node.setBufferNonces(buffer);                               // send them to LoRaWAN
    debug(state != RADIOLIB_ERR_NONE, F("Restoring nonces buffer failed"), state, false);

    // recall session from RTC deep-sleep preserved variable
    state = node.setBufferSession(LWsession); // send them to LoRaWAN stack

    // if we have booted more than once we should have a session to restore, so report any failure
    // otherwise no point saying there's been a failure when it was bound to fail with an empty LWsession var.
    debug((state != RADIOLIB_ERR_NONE) && (bootCount > 1), F("Restoring session buffer failed"), state, false);

    // if Nonces and Session restored successfully, activation is just a formality
    // moreover, Nonces didn't change so no need to re-save them
    if (state == RADIOLIB_ERR_NONE)
    {
      console.log(sqINFO, F("Succesfully restored session - now activating"));
      state = active_node();
      debug((state != RADIOLIB_LORAWAN_SESSION_RESTORED), F("Failed to activate restored session"), state, true);
      // ##### close the store before returning
      store.end();
      return (state);
    }
  }
  else
  { // store has no key "nonces"
    console.log(sqINFO, F("No Nonces saved - starting fresh."));
  }

  // if we got here, there was no session to restore, so start trying to join
  state = RADIOLIB_ERR_NETWORK_NOT_JOINED;
  while (state != RADIOLIB_LORAWAN_NEW_SESSION)
  {
    console.log(sqINFO, F("Join ('login') to the LoRaWAN Network"));
    state = active_node();
    // ##### save the join counters (nonces) to permanent store
    console.log(sqINFO, F("Saving nonces to flash"));
    uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
    uint8_t *persist = node.getBufferNonces();                          // get pointer to nonces
    memcpy(buffer, persist, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);          // copy in to buffer
    store.putBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // send them to the store
    // we'll save the session after an uplink
    if (state != RADIOLIB_LORAWAN_NEW_SESSION)
    {
      console.log(sqERROR, "Join failed: %d", state);
      // how long to wait before join attempts. This is an interim solution pending
      // implementation of TS001 LoRaWAN Specification section #7 - this doc applies to v1.0.4 & v1.1
      // it sleeps for longer & longer durations to give time for any gateway issues to resolve
      // or whatever is interfering with the device <-> gateway airwaves.
      uint32_t sleepMs = min((bootCountSinceUnsuccessfulJoin++ + 1UL) * 60UL, 3UL * 60UL * 1000UL);
      console.log(sqERROR, "Boots since unsuccessful join: %d Retrying join in %ld milliseconds", bootCountSinceUnsuccessfulJoin, sleepMs);
      gotoSleep(sleepMs);
    }
  } // while join
  console.log(sqINFO, F("Joined"));
  setParams(params);
  // reset the failed join count
  bootCountSinceUnsuccessfulJoin = 0;
  delay(1000); // hold off off hitting the airwaves again too soon - an issue in the US
  // ##### close the store
  store.end();
  return (state);
}
// Begin method: Initializes the LoRaWAN service
bool LoRaWanService::begin(bool autogen)
{
  int16_t state = 0; // return value for calls to RadioLib

  console.log(sqINFO, F("Initalise the radio"));
  console.log(sqINFO, "Reset SX1262 ");
  Serial.print("Reset SX1262 ");
  pinMode(LORA_NRST, OUTPUT);
  digitalWrite(LORA_NRST, LOW);
  delay(1);
  digitalWrite(LORA_NRST, HIGH);
  delay(1);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  digitalWrite(LORA_NSS, LOW);

  // 等 BUSY 釋放
  pinMode(LORA_BUSY, INPUT);
  while (digitalRead(LORA_BUSY))
  {
    Serial.print(".");
    Serial.flush();
  }
  Serial.println(" done.");
  state = radio.begin();

  if (state == RADIOLIB_ERR_NONE)
  {
    console.log(sqINFO, F("Radio initialised OK"));
  }
  else
  {
    debug(true, F("Initialise radio failed"), state, true);
  }

  if (autogen)
  {
    console.log(sqINFO, F("Generating keys"));
    autoGenKeys();
  }

  setParams(params);
  // activate node by restoring session or otherwise joining the network
  state = lwActivate();

  return (state == RADIOLIB_LORAWAN_NEW_SESSION ||
          state == RADIOLIB_LORAWAN_SESSION_RESTORED);
}

// Stop method: Stops the LoRaWAN service
void LoRaWanService::stop()
{
  radio.sleep();
  SPI.end();
}

void LoRaWanService::softSleep()
{
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  radio.begin();
  delay(10);
  radio.sleep();
  SPI.end();
}

void LoRaWanService::sleep(enum LORAWAN_SLEEP_TYPE sleep_type)
{
  uint32_t delayMs = params->uplinkIntervalSeconds * 1000;
  switch (sleep_type)
  {
  case LORAWAN_SLEEP_NONE:
    SPI.end();
    break;
  case LORAWAN_SLEEP_IN_RADIO_OFF:
    radio.sleep();
    break;
  case LORAWAN_SLEEP_IN_DEEP:
    radio.sleep();
    SPI.end();
    gotoSleep(delayMs);
    break;
  case LORAWAN_SLEEP_IN_DEEP_WITH_TTN_LAW:
    radio.sleep();
    SPI.end();
    uint32_t minimumDelay = delayMs;
    uint32_t interval = node.timeUntilUplink(); // calculate minimum duty cycle delay (per FUP & law!)
    delayMs = max(interval, minimumDelay);      // cannot send faster than duty cycle allows
    gotoSleep(delayMs);
    break;
  }
}

// Set battery level (for use in uplink messages or ADR)
void LoRaWanService::set_battery_level(int level)
{
  node.setDeviceStatus(level);
}

// Publish a message
void LoRaWanService::send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, uint8_t *fPortDown, bool isConfirmed)
{
  int16_t state = 0;
  console.log(sqINFO, "send process start");

  // you can also retrieve additional information about an uplink or
  // downlink by passing a reference to LoRaWANEvent_t structure
  LoRaWANEvent_t uplinkDetails;
  LoRaWANEvent_t downlinkDetails;

  // Retrieve the last uplink frame counter
  uint32_t fCntUp = node.getFCntUp();
  // Send a confirmed uplink on the second uplink
  // and also request the LinkCheck and DeviceTime MAC commands
  if (fCntUp == 1)
  {
    console.log(sqINFO, F("Sending uplink and requesting LinkCheck and DeviceTime"));
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_LINK_CHECK);
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_DEVICE_TIME);
    state = node.sendReceive(dataUp, lenUp, fPortUp, dataDown, lenDown, isConfirmed, &uplinkDetails, &downlinkDetails);
  }
  else
  {
    state = node.sendReceive(dataUp, lenUp, fPortUp, dataDown, lenDown, isConfirmed, &uplinkDetails, &downlinkDetails);
  }
  debug((state < RADIOLIB_ERR_NONE) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);
  fCntUp = node.getFCntUp();
  console.log(sqINFO, "Sending uplink FCNT: %d", fCntUp);
  // now save session to RTC memory
  uint8_t *persist = node.getBufferSession();
  memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

  // Check if a downlink was received
  // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
  success_downlink_received = false;
  if (state > 0)
  {
    console.log(sqINFO, "Received a downlink on port %d", downlinkDetails.fPort);
    if (fPortDown != NULL)
    {
      *fPortDown = downlinkDetails.fPort;
    }
    success_downlink_received = true;
  }
  else
  {
    console.log(sqINFO, F("No downlink received"));
  }
}

void LoRaWanService::send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, bool isConfirmed)
{
  send_and_receive(dataUp, lenUp, fPortUp, dataDown, lenDown, NULL, isConfirmed);
}

void LoRaWanService::send_and_receive(bool *success, const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, uint8_t *fPortDown, bool isConfirmed)
{
  send_and_receive(dataUp, lenUp, fPortUp, dataDown, lenDown, fPortDown, isConfirmed);
  *success = success_downlink_received;
}
void LoRaWanService::send_and_receive(bool *success, const uint8_t *dataUp, size_t lenUp, uint8_t fPortUp, uint8_t *dataDown, size_t *lenDown, bool isConfirmed)
{
  send_and_receive(success, dataUp, lenUp, fPortUp, dataDown, lenDown, NULL, isConfirmed);
}

void LoRaWanService::setCSMA(bool csmaEnabled, uint8_t maxChanges, uint8_t backoffMax, uint8_t difsSlots)
{
  node.setCSMA(csmaEnabled, maxChanges, backoffMax, difsSlots);
}

void LoRaWanService::setClass(LoRaWANClass cls)
{
  node.setClass(cls);
}

void LoRaWanService::startClassC()
{
  node.startClassC();
}

void LoRaWanService::stopClassC()
{
  node.stopClassC();
}

bool LoRaWanService::pollClassC(uint8_t *dataDown, size_t *lenDown, uint8_t *portDown) // 在主循環呼叫：若 ISR 置位，取包並解析
{
  return node.pollClassC(dataDown, lenDown, portDown);
}

void LoRaWanService::taskLoop(void *param)
{
  static uint32_t lastMillis = 0;
  LoRaWanService *instance = static_cast<LoRaWanService *>(param);

  uint8_t downBuffer[255];
  size_t downLen = 0;
  uint8_t downPort = 0;
  LoraUplinkMsg_t updata;
  for (;;)
  {
    uint32_t now = millis();

    // 範例：at least interval uplink
    if ((now - lastMillis >= instance->uplink_interval_ms()) && (!instance->is_uplink_queue_empty()))
    {
      lastMillis = now;
      instance->pop_from_uplink_queue(&updata);
      instance->set_battery_level(updata.battery_level);
      instance->send_and_receive(
          (const uint8_t *)updata.data, updata.len,
          updata.fport,
          downBuffer, &downLen, &downPort,
          updata.isConfirmed);
      // 這裡可能也會帶回下行（視 NS 行為），一樣往下交給 pollClassC 流程
      if (downLen > 0)
      {
        Serial.println("Downlink received in taskLoop after uplink");
        print_bytes(downBuffer, downLen);
        instance->push_to_downlink_queue(downBuffer, downLen, downPort);
        downLen = 0;
        downPort = 0;
        memset(downBuffer, 0, 255);
      }
    }

    // 維持 Class C 常駐接收；若抓到下行，複製到 queue
    if (instance->pollClassC(downBuffer, &downLen, &downPort))
    {
      if (downLen > 0)
      {
        Serial.println("Class C downlink received in taskLoop");
        instance->push_to_downlink_queue(downBuffer, downLen, downPort);
        downLen = 0;
        downPort = 0;
        memset(downBuffer, 0, 255);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// result code to text - these are error codes that can be raised when using LoRaWAN
// however, RadioLib has many more - see https://jgromes.github.io/RadioLib/group__status__codes.html for a complete list
String stateDecode(const int16_t result)
{
  switch (result)
  {
  case RADIOLIB_ERR_NONE:
    return "ERR_NONE";
  case RADIOLIB_ERR_CHIP_NOT_FOUND:
    return "ERR_CHIP_NOT_FOUND";
  case RADIOLIB_ERR_PACKET_TOO_LONG:
    return "ERR_PACKET_TOO_LONG";
  case RADIOLIB_ERR_RX_TIMEOUT:
    return "ERR_RX_TIMEOUT";
  case RADIOLIB_ERR_CRC_MISMATCH:
    return "ERR_CRC_MISMATCH";
  case RADIOLIB_ERR_INVALID_BANDWIDTH:
    return "ERR_INVALID_BANDWIDTH";
  case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
    return "ERR_INVALID_SPREADING_FACTOR";
  case RADIOLIB_ERR_INVALID_CODING_RATE:
    return "ERR_INVALID_CODING_RATE";
  case RADIOLIB_ERR_INVALID_FREQUENCY:
    return "ERR_INVALID_FREQUENCY";
  case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
    return "ERR_INVALID_OUTPUT_POWER";
  case RADIOLIB_ERR_NETWORK_NOT_JOINED:
    return "RADIOLIB_ERR_NETWORK_NOT_JOINED";
  case RADIOLIB_ERR_DOWNLINK_MALFORMED:
    return "RADIOLIB_ERR_DOWNLINK_MALFORMED";
  case RADIOLIB_ERR_INVALID_REVISION:
    return "RADIOLIB_ERR_INVALID_REVISION";
  case RADIOLIB_ERR_INVALID_PORT:
    return "RADIOLIB_ERR_INVALID_PORT";
  case RADIOLIB_ERR_NO_RX_WINDOW:
    return "RADIOLIB_ERR_NO_RX_WINDOW";
  case RADIOLIB_ERR_INVALID_CID:
    return "RADIOLIB_ERR_INVALID_CID";
  case RADIOLIB_ERR_UPLINK_UNAVAILABLE:
    return "RADIOLIB_ERR_UPLINK_UNAVAILABLE";
  case RADIOLIB_ERR_COMMAND_QUEUE_FULL:
    return "RADIOLIB_ERR_COMMAND_QUEUE_FULL";
  case RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND:
    return "RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND";
  case RADIOLIB_ERR_JOIN_NONCE_INVALID:
    return "RADIOLIB_ERR_JOIN_NONCE_INVALID";
  case RADIOLIB_ERR_N_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_N_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_A_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_A_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
    return "RADIOLIB_ERR_DWELL_TIME_EXCEEDED";
  case RADIOLIB_ERR_CHECKSUM_MISMATCH:
    return "RADIOLIB_ERR_CHECKSUM_MISMATCH";
  case RADIOLIB_ERR_NO_JOIN_ACCEPT:
    return "RADIOLIB_ERR_NO_JOIN_ACCEPT";
  case RADIOLIB_LORAWAN_SESSION_RESTORED:
    return "RADIOLIB_LORAWAN_SESSION_RESTORED";
  case RADIOLIB_LORAWAN_NEW_SESSION:
    return "RADIOLIB_LORAWAN_NEW_SESSION";
  case RADIOLIB_ERR_NONCES_DISCARDED:
    return "RADIOLIB_ERR_NONCES_DISCARDED";
  case RADIOLIB_ERR_SESSION_DISCARDED:
    return "RADIOLIB_ERR_SESSION_DISCARDED";
  }
  return "See https://jgromes.github.io/RadioLib/group__status__codes.html";
}

// helper function to display any issues
void debug(bool failed, const __FlashStringHelper *message, int state, bool halt)
{
  if (failed)
  {
    Serial.print(message);
    Serial.print(" - ");
    Serial.print(stateDecode(state));
    Serial.print(" (");
    Serial.print(state);
    Serial.println(")");
    while (halt)
    {
      delay(1);
    }
  }
}

void print_bytes(uint8_t *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    // Serial.print(" ");
  }
  Serial.println();
}

void print_bytes_reverse(uint8_t *data, int length)
{
  for (int i = length - 1; i >= 0; i--)
  {
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    // Serial.print(" ");
  }
  Serial.println();
}

#endif
