#include "bsp.h"
#ifdef USE_LORAWAN // Only compile when USE_LORAWAN is enabled
#include "lorawan_service.h"
#include "radiolab/src/RadioLib.h"
#include <Preferences.h>
Preferences store;
RTC_DATA_ATTR uint16_t bootCount = 0;
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
  print_bytes((uint8_t *)&params->DEVEUI, sizeof(params->DEVEUI));
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

int LoRaWanService::recall_nonces()
{
  int16_t state = RADIOLIB_ERR_NONE;
  console.log(sqINFO, "Recalling LoRaWAN nonces & session");
  // ##### setup the flash storage
  store.begin("radiolib");
  // ##### if we have previously saved nonces, restore them and try to restore session as well
  if (store.isKey("nonces"))
  {
    uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
    store.getBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // get them from the store

    // console.log(sqINFO, "Restoring nonces:");
    // for (int i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++)
    // {
    //   Serial.print(buffer[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println();

    state = node.setBufferNonces(buffer); // send them to LoRaWAN
    debug(state != RADIOLIB_ERR_NONE, F("Restoring nonces buffer failed"), state, false);

    // recall session from RTC deep-sleep preserved variable
    state = node.setBufferSession(LWsession); // send them to LoRaWAN stack

    // if we have booted more than once we should have a session to restore, so report any failure
    // otherwise no point saying there's been a failure when it was bound to fail with an empty LWsession var.
    debug((state != RADIOLIB_ERR_NONE) && (bootCount > 1), F("Restoring session buffer failed"), state, false);
  }
  else
  { // store has no key "nonces"
    console.log(sqINFO, "No Nonces saved - starting fresh.");
    state = RADIOLIB_ERR_NONCES_DISCARDED;
  }
  // ##### close the store before returning
  store.end();
  return state;
}

// Begin method: Initializes the LoRaWAN service
bool LoRaWanService::begin(bool autogen)
{
  SPI.begin(params->SCK, params->MISO, params->MOSI, params->NSS); // Initialize SPI
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);
  console.log(sqINFO, F("Initialise LoRaWAN Network credentials"));
  if (autogen)
  {
    autoGenKeys();
  }
  state = begin_node();
  debug(state != RADIOLIB_ERR_NONE, F("Begin node failed"), state, true);
  state = recall_nonces();
  debug(state != RADIOLIB_ERR_NONE, F("Recalling nonces failed,then renew a session"), state, false);
  if (state != RADIOLIB_ERR_NONE)
  {
    state = RADIOLIB_ERR_NETWORK_NOT_JOINED;
    store.begin("radiolib");
    while (state != RADIOLIB_LORAWAN_NEW_SESSION)
    {
      state = active_node();
      // ##### save the join counters (nonces) to permanent store
      console.log(sqINFO, "Saving nonces to flash");
      uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
      uint8_t *persist = node.getBufferNonces();                          // get pointer to nonces
      memcpy(buffer, persist, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);          // copy in to buffer
      store.putBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // send them to the store
      // Serial.print("Nonces: ");
      // for (int i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++)
      // {
      //   Serial.print(buffer[i], HEX);
      //   Serial.print(" ");
      // }
      // Serial.println("check again");
      // uint8_t buffer2[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
      // store.getBytes("nonces", buffer2, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // get them from the store
      // for (int i = 0; i < RADIOLIB_LORAWAN_NONCES_BUF_SIZE; i++)
      // {
      //   Serial.print(buffer2[i], HEX);
      //   Serial.print(" ");
      // }
      store.end();
      if (state != RADIOLIB_LORAWAN_NEW_SESSION)
      {
        console.log(sqINFO, "Join failed: %d", state);
        // how long to wait before join attempts. This is an interim solution pending
        // implementation of TS001 LoRaWAN Specification section #7 - this doc applies to v1.0.4 & v1.1
        // it sleeps for longer & longer durations to give time for any gateway issues to resolve
        // or whatever is interfering with the device <-> gateway airwaves.
        uint32_t sleepForSeconds = min((bootCountSinceUnsuccessfulJoin++ + 1UL) * 60UL, 3UL * 60UL);
        console.log(sqINFO, "Boots since unsuccessful join: %d", bootCountSinceUnsuccessfulJoin);
        console.log(sqINFO, "Retrying join in %d seconds", sleepForSeconds);
        gotoSleep(sleepForSeconds);
      }
    }
  }
  console.log(sqINFO, F("Joined"));
  // reset the failed join count
  bootCountSinceUnsuccessfulJoin = 0;
  delay(1000); // hold off off hitting the airwaves again too soon - an issue in the US
  setParams();

  // Serial.println(F("Recalling LoRaWAN nonces & session"));
  // // ##### setup the flash storage
  // store.begin("radiolib");
  // // ##### if we have previously saved nonces, restore them and try to restore session as well
  // if (store.isKey("nonces"))
  // {
  //   uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
  //   store.getBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // get them from the store
  //   state = node.setBufferNonces(buffer);                               // send them to LoRaWAN
  //   debug(state != RADIOLIB_ERR_NONE, F("Restoring nonces buffer failed"), state, false);

  //   // recall session from RTC deep-sleep preserved variable
  //   state = node.setBufferSession(LWsession); // send them to LoRaWAN stack

  //   // if we have booted more than once we should have a session to restore, so report any failure
  //   // otherwise no point saying there's been a failure when it was bound to fail with an empty LWsession var.
  //   debug((state != RADIOLIB_ERR_NONE) && (bootCount > 1), F("Restoring session buffer failed"), state, false);

  //   // if Nonces and Session restored successfully, activation is just a formality
  //   // moreover, Nonces didn't change so no need to re-save them
  //   if (state == RADIOLIB_ERR_NONE)
  //   {
  //     Serial.println(F("Succesfully restored session - now activating"));
  //     state = node.activateOTAA();
  //     debug((state != RADIOLIB_LORAWAN_SESSION_RESTORED), F("Failed to activate restored session"), state, true);

  //     // ##### close the store before returning
  //     store.end();
  //     // return (state);
  //   }
  // }
  // else
  // { // store has no key "nonces"
  //   Serial.println(F("No Nonces saved - starting fresh."));

  //   // if we got here, there was no session to restore, so start trying to join
  //   state = RADIOLIB_ERR_NETWORK_NOT_JOINED;
  //   while (state != RADIOLIB_LORAWAN_NEW_SESSION)
  //   {
  //     Serial.println(F("Join ('login') to the LoRaWAN Network"));
  //     if (params->OTAA)
  //     {
  //       uint8_t joinDR = 4;
  //       state = node.activateOTAA(joinDR);
  //     }
  //     else
  //     {
  //       state = node.activateABP();
  //     }

  //     // ##### save the join counters (nonces) to permanent store
  //     Serial.println(F("Saving nonces to flash"));
  //     uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];                   // create somewhere to store nonces
  //     uint8_t *persist = node.getBufferNonces();                          // get pointer to nonces
  //     memcpy(buffer, persist, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);          // copy in to buffer
  //     store.putBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE); // send them to the store

  //     // we'll save the session after an uplink

  //   if (state != RADIOLIB_LORAWAN_NEW_SESSION)
  //   {
  //     Serial.print(F("Join failed: "));
  //     Serial.println(state);

  //     // how long to wait before join attempts. This is an interim solution pending
  //     // implementation of TS001 LoRaWAN Specification section #7 - this doc applies to v1.0.4 & v1.1
  //     // it sleeps for longer & longer durations to give time for any gateway issues to resolve
  //     // or whatever is interfering with the device <-> gateway airwaves.
  //     uint32_t sleepForSeconds = min((bootCountSinceUnsuccessfulJoin++ + 1UL) * 60UL, 3UL * 60UL);
  //     Serial.print(F("Boots since unsuccessful join: "));
  //     Serial.println(bootCountSinceUnsuccessfulJoin);
  //     Serial.print(F("Retrying join in "));
  //     Serial.print(sleepForSeconds);
  //     Serial.println(F(" seconds"));
  //     delay(1000);
  //     // gotoSleep(sleepForSeconds);
  //   } // if activateOTAA state
  // } // while join
  // Serial.println(F("Joined"));
  // // reset the failed join count
  // bootCountSinceUnsuccessfulJoin = 0;
  // delay(1000); // hold off off hitting the airwaves again too soon - an issue in the US
  //              // ##### close the store
  // store.end();
  // if (params->OTAA)
  // {
  //   uint8_t joinDR = 4;
  //   state = node.activateOTAA(joinDR);
  //   debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, false);
  // }
  // else
  // {
  //   state = node.activateABP();
  //   debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Activate ABP failed"), state, false);
  // }
  // if (state != RADIOLIB_LORAWAN_NEW_SESSION)
  // {
  //   Serial.println(F("Initialise LoRaWAN Network credentials failed"));
  //   return false;
  // }
  // }
  // Print the DevAddr
  return true;
}

// Stop method: Stops the LoRaWAN service
void LoRaWanService::stop()
{
  // Logic to stop the radio
}

void LoRaWanService::sleep()
{
  radio.sleep();
  SPI.end();
}

// Set battery level (for use in uplink messages or ADR)
void LoRaWanService::set_battery_level(int level)
{
  node.setDeviceStatus(level);
}

// Publish a message
void LoRaWanService::send_and_receive(const uint8_t *dataUp, size_t lenUp, uint8_t fPort, uint8_t *dataDown, size_t *lenDown, bool isConfirmed)
{
  console.log(sqINFO, "send process start");
  // you can also retrieve additional information about an uplink or
  // downlink by passing a reference to LoRaWANEvent_t structure
  LoRaWANEvent_t uplinkDetails;
  LoRaWANEvent_t downlinkDetails;

  // Retrieve the last uplink frame counter
  uint32_t fCntUp = node.getFCntUp();
  // Send a confirmed uplink on the second uplink
  // and also request the LinkCheck and DeviceTime MAC commands
  console.log(sqINFO, "Sending uplink FCNT: %d", fCntUp);
  uint16_t state = 0;
  if (fCntUp == 1)
  {
    console.log(sqINFO, F("Sending uplink and requesting LinkCheck and DeviceTime"));
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_LINK_CHECK);
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_DEVICE_TIME);
    state = node.sendReceive(dataUp, lenUp, fPort, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }
  else
  {
    state = node.sendReceive(dataUp, lenUp, fPort, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }
  debug((state < RADIOLIB_ERR_NONE) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);

  // now save session to RTC memory
  uint8_t *persist = node.getBufferSession();
  memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);

  // Check if a downlink was received
  // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
  if (state > 0)
  {
    console.log(sqINFO, F("Received a downlink"));
  }
  else
  {
    console.log(sqINFO, F("No downlink received"));
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
