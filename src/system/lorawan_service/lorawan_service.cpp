#include "bsp.h"
#include "lorawan_service.h"

#ifdef USE_LORAWAN // Only compile when USE_LORAWAN is enabled
#include "LoRaWan_APP.h"
/* OTAA para*/
uint8_t devEui[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};

/* ABP para*/
uint8_t nwkSKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x02, 0x02};
uint8_t appSKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x02, 0x02};
uint32_t devAddr = (uint32_t)0x88888888;
/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
/*LoraWan region,
    LORAMAC_REGION_AS923        //  AS band on 923MHz
    LORAMAC_REGION_AU915        //  Australian band on 915MHz
    LORAMAC_REGION_CN470        //  Chinese band on 470MHz
    LORAMAC_REGION_CN779        //  Chinese band on 779MHz
    LORAMAC_REGION_EU433        //  European band on 433MHz
    LORAMAC_REGION_EU868        //  European band on 868MHz
    LORAMAC_REGION_KR920        //  South korean band on 920MHz
    LORAMAC_REGION_IN865        //  India band on 865MHz
    LORAMAC_REGION_US915        //  North american band on 915MHz
    LORAMAC_REGION_US915_HYBRID //  North american band on 915MHz with a maximum of 16 channels
    LORAMAC_REGION_AU915_SB2    //  Australian band on 915MHz Subband 2
    LORAMAC_REGION_AS923_AS1    //  AS band on 922.0-923.4MHz
    LORAMAC_REGION_AS923_AS2    //  AS band on 923.2-924.6MHz
*/
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_AS923_AS2;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_C;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = false;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
 * Number of trials to transmit the frame, if the LoRaMAC layer did not
 * receive an acknowledgment. The MAC performs a datarate adaptation,
 * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
 * to the following table:
 *
 * Transmission nb | Data Rate
 * ----------------|-----------
 * 1 (first)       | DR
 * 2               | DR
 * 3               | max(DR-1,0)
 * 4               | max(DR-1,0)
 * 5               | max(DR-2,0)
 * 6               | max(DR-2,0)
 * 7               | max(DR-3,0)
 * 8               | max(DR-3,0)
 *
 * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
 * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
 */
uint8_t confirmedNbTrials = 4;

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

void generate_lorawan_settings_by_chip_id()
{
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("ESP32ChipID=%04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  devAddr = (uint32_t)(chipid >> 32) * (uint32_t)chipid;
  // 将MAC地址转换为字符串形式
  char chipidStr[17];
  snprintf(chipidStr, sizeof(chipidStr), "%016llx", chipid);

  Serial.print("devEUI:");
  memcpy(&devEui[2], &chipid, sizeof(devEui) - 2);
  print_bytes((uint8_t *)&devEui, sizeof(devEui));
  Serial.print("devAddr:");
  print_bytes_reverse((uint8_t *)&devAddr, sizeof(devAddr));
  memcpy(appKey, chipidStr, 16);
  memcpy(appSKey, chipidStr, 16);
  memcpy(nwkSKey, chipidStr, 16);
  Serial.print("appKey:");
  print_bytes((uint8_t *)&appKey, sizeof(appKey));
  Serial.print("nwkSKey:");
  print_bytes((uint8_t *)&nwkSKey, sizeof(nwkSKey));
  Serial.print("appSKey:");
  print_bytes((uint8_t *)&appSKey, sizeof(appSKey));
}

// Constructor
SQ_LoRaWanService::SQ_LoRaWanService()
{
  // Initially, no task is running
  loraTaskHandle = nullptr;
  generate_lorawan_settings_by_chip_id();
}

// Destructor
SQ_LoRaWanService::~SQ_LoRaWanService()
{
  // Ensure the task is stopped and deleted before destruction
  stopTask();
}

// Task function
void SQ_LoRaWanService::LoRaWanTaskFunction(void *pvParameters)
{
  // Cast the parameter to an instance of SQ_LoRaWanService
  SQ_LoRaWanService *loraService = static_cast<SQ_LoRaWanService *>(pvParameters);
  deviceState = DEVICE_STATE_INIT;
  // Task loop
  while (true)
  {
    // Add your LoRaWAN processing code here
    switch (deviceState)
    {
    case DEVICE_STATE_INIT:
    {
      LoRaWAN.init(loraWanClass, loraWanRegion);
      // both set join DR and DR when ADR off
      if (!loraWanAdr)
      {
        LoRaWAN.setDefaultDR(3);
      }
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep(loraWanClass);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Method to start the task
void SQ_LoRaWanService::startTask()
{
  if (loraTaskHandle == nullptr)
  { // Ensure the task is not already running
    xTaskCreate(
        LoRaWanTaskFunction, // Task function
        "LoRaWAN Task",      // Task name
        4096,                // Stack size (in words)
        this,                // Pass the instance of the class as the parameter
        5,                   // Task priority
        &loraTaskHandle      // Task handle
    );
  }
}

// Method to stop the task
void SQ_LoRaWanService::stopTask()
{
  if (loraTaskHandle != nullptr)
  {
    vTaskDelete(loraTaskHandle); // Delete the task
    loraTaskHandle = nullptr;    // Reset the task handle to indicate the task is stopped
  }
}

// Create an instance of the LoRaWAN service class
SQ_LoRaWanService loraWanService;
#endif // USE_LORAWAN