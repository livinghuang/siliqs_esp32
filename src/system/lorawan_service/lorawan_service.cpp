#include "bsp.h"
#ifdef USE_LORAWAN // Only compile when USE_LORAWAN is enabled
#include "lorawan_service.h"
#include "radiolab/src/RadioLib.h"
const LoRaWANBand_t Region = REGION;
const uint8_t subBand = SUB_BAND; // For US915, change this to 2, otherwise leave on 0

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
LoRaWANNode node(&radio, &Region, subBand);
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

// Begin method: Initializes the LoRaWAN service
bool LoRaWanService::begin(bool autogen)
{
  SPI.begin(params->SCK, params->MISO, params->MOSI, params->NSS); // Initialize SPI
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);

  console.log(sqINFO, F("Initialise LoRaWAN Network credentials"));

  if (autogen)
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
      console.log(sqINFO, "LoRaWAN 1.1");
      node.beginOTAA(params->JOINEUI, params->DEVEUI, params->NWKxKEY, params->APPxKEY);
    }
    else
    {
      console.log(sqINFO, "LoRaWAN 1.0.4");
      node.beginOTAA(params->JOINEUI, params->DEVEUI, NULL, params->APPxKEY);
    }
    uint8_t joinDR = 4;
    state = node.activateOTAA(joinDR);
    debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, true);
  }
  else
  {
    if (params->LORAWAN_1_1 == true)
    {
      node.beginABP(params->DEVADDR, params->FNWKSINT, params->SNWKSINT, params->NWKxKEY, params->APPxKEY);
    }
    else
    {
      node.beginABP(params->DEVADDR, NULL, NULL, params->NWKxKEY, params->APPxKEY);
    }
    node.activateABP();
    debug(state != RADIOLIB_ERR_NONE, F("Activate ABP failed"), state, true);
  }

  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.println(F("Initialise LoRaWAN Network credentials failed"));
    return false;
  }
  // Print the DevAddr
  Serial.print("[LoRaWAN] DevAddr: ");
  Serial.println((unsigned long)node.getDevAddr(), HEX);

  // Enable the ADR algorithm (on by default which is preferable)
  node.setADR(params->ADR);

  // Set a datarate to start off with
  node.setDatarate(params->DR);

  // / Duty Cycle = 1 / (DutyCycleFactor) , if 0, disable. In EU law, Duty Cycle should under 1%
  node.setDutyCycle(params->DutyCycleFactor ? true : false, params->DutyCycleFactor);

  // Unit: ms, Dwell Time to limit signal airtime in single channel, In US/AU law,Dwell Time under 400ms
  node.setDwellTime(params->DwellTime ? true : false, params->DwellTime);

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
  console.log(sqINFO, F("Sending uplink"));
  uint16_t state = 0;
  if (fCntUp == 1)
  {
    console.log(sqINFO, F("and requesting LinkCheck and DeviceTime"));
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_LINK_CHECK);
    node.sendMacCommandReq(RADIOLIB_LORAWAN_MAC_DEVICE_TIME);
    state = node.sendReceive(dataUp, lenUp, fPort, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }
  else
  {
    state = node.sendReceive(dataUp, lenUp, fPort, dataDown, lenDown, true, &uplinkDetails, &downlinkDetails);
  }

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
