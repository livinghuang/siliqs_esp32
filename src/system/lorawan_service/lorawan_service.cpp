#include "bsp.h"
#ifdef USE_LORAWAN // Only compile when USE_LORAWAN is enabled
#include "lorawan_service.h"
#include "radiolab/src/RadioLib.h"
#include <Preferences.h>
Preferences store;
RTC_DATA_ATTR uint16_t bootCountSinceUnsuccessfulJoin = 0;
RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];

const LoRaWANBand_t Region = REGION;
const uint8_t subBand = SUB_BAND; // For US915, change this to 2, otherwise leave on 0

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
RTC_DATA_ATTR LoRaWANNode node(&radio, &Region, subBand);
void print_bytes_reverse(uint8_t *data, int length);
void print_bytes(uint8_t *data, int length);
// Constructor
LoRaWanService::LoRaWanService(lorawan_params_settings *params)
{
  this->params = params;
  pinMode(LORA_NRST, OUTPUT);
  digitalWrite(LORA_NRST, LOW);
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

void LoRaWanService::setParams(void)
{
  int16_t state = RADIOLIB_ERR_NONE;
  console.log(sqINFO, "DevAddr: ");
  Serial.println((unsigned long)node.getDevAddr(), HEX);
  // Enable the ADR algorithm (on by default which is preferable)
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
  setParams();
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
  // setup the radio based on the pinmap (connections) in config.h
  console.log(sqINFO, F("Initalise the radio"));
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initalise radio failed"), state, true);
  if (autogen == true)
  {
    console.log(sqINFO, F("Generating keys"));
    autoGenKeys();
  }
  // activate node by restoring session or otherwise joining the network
  state = lwActivate();
  // state is one of RADIOLIB_LORAWAN_NEW_SESSION or RADIOLIB_LORAWAN_SESSION_RESTORED
  return true;
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
    state = node.sendReceive(dataUp, lenUp, fPortUp, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }
  else
  {
    state = node.sendReceive(dataUp, lenUp, fPortUp, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }
  debug((state < RADIOLIB_ERR_NONE) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);
  fCntUp = node.getFCntUp();
  console.log(sqINFO, "Sending uplink FCNT: %d", fCntUp);
  // now save session to RTC memory
  uint8_t *persist = node.getBufferSession();
  memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

  // Check if a downlink was received
  // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
  if (state > 0)
  {
    console.log(sqINFO, "Received a downlink on port %d", downlinkDetails.fPort);
    if (fPortDown != NULL)
    {
      *fPortDown = downlinkDetails.fPort;
    }
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
