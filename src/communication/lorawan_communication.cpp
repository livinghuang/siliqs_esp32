// #include "bsp.h"
// #ifdef USE_LORAWAN
// #include "lorawan_communication.h"

// // 全域變數初始化
// uint8_t devEui[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// uint8_t appEui[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// uint8_t appKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};

// uint8_t nwkSKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// uint8_t appSKey[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
// uint32_t devAddr = (uint32_t)0x88888888;

// uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
// LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_AS923_AS2;
// DeviceClass_t loraWanClass = CLASS_A;
// uint32_t appTxDutyCycle = 15000;
// bool overTheAirActivation = false;
// bool loraWanAdr = true;
// bool isTxConfirmed = false;
// uint8_t appPort = 2;
// uint8_t confirmedNbTrials = 4;
// void print_bytes(const uint8_t *data, int length)
// {
//   for (int i = 0; i < length; i++)
//   {
//     // Print each byte in hexadecimal format with leading zeros
//     if (data[i] < 0x10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(data[i], HEX);
//     Serial.print(" ");
//   }
//   Serial.println(); // Print a newline character at the end
// }
// void print_bytes_reverse(uint8_t *data, int length)
// {
//   for (int i = length - 1; i >= 0; i--)
//   {
//     if (data[i] < 0x10)
//     {
//       Serial.print("0");
//     }
//     Serial.print(data[i], HEX);
//     // Serial.print(" ");
//   }
//   Serial.println();
// }
// // Constructor
// SQ_LoRaWanClass::SQ_LoRaWanClass()
// {
//   // You can add custom initialization code here if necessary
//   console.log(sqINFO, "SQ_LoRaWanClass: Constructor called.");
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_Init(DeviceClass_t lorawanClass, LoRaMacRegion_t region)
// {
//   // Call the base class initialization method
//   LoRaWanClass::init(lorawanClass, region);

//   // Additional custom initialization logic
//   console.log(sqINFO, "SQ_LoRaWanClass: Initialization complete.");
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_send()
// {
//   // Override the base class send method
//   LoRaWanClass::send(); // Call the parent class's send() method

//   // Additional functionality after sending data
//   console.log(sqINFO, "SQ_LoRaWanClass: Data sent successfully.");
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_SendData(uint8_t *data, uint8_t size)
// {
//   // Prepare the data to send over LoRaWAN
//   appDataSize = size;
//   memcpy(appData, data, size); // Copy the data to the buffer

//   // Set the device state to send data
//   deviceState = DEVICE_STATE_SEND;

//   // Log the data sending process
//   console.log(sqINFO, "SQ_LoRaWanClass: Sending data of size " + String(size));
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_Cycle(uint32_t dutyCycle)
// {
//   // Call the base class cycle method
//   LoRaWanClass::cycle(dutyCycle);

//   // Additional logic if needed
//   console.log(sqINFO, "SQ_LoRaWanClass: Cycling with duty cycle " + String(dutyCycle));
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_Sleep(DeviceClass_t classMode)
// {
//   // Call the base class sleep method
//   LoRaWanClass::sleep(classMode);

//   // Additional logic for sleep mode if necessary
//   console.log(sqINFO, "SQ_LoRaWanClass: Device entering sleep mode.");
// }

// void generate_lorawan_settings_by_chip_id()
// {
//   uint64_t chipid = ESP.getEfuseMac();
//   devAddr = (uint32_t)(chipid >> 32) * (uint32_t)chipid;
//   // 将MAC地址转换为字符串形式
//   char chipidStr[17];
//   snprintf(chipidStr, sizeof(chipidStr), "%016llx", chipid);

//   console.log(sqINFO, "devEUI:");
//   memcpy(&devEui[2], &chipid, sizeof(devEui) - 2);
//   console.log(sqINFO, (uint8_t *)&devEui, sizeof(devEui));
//   console.log(sqINFO, "devAddr:");
//   console.log(sqINFO, (uint8_t *)&devAddr, sizeof(devAddr));
//   memcpy(appKey, chipidStr, 16);
//   memcpy(appSKey, chipidStr, 16);
//   memcpy(nwkSKey, chipidStr, 16);
//   Serial.print("appKey:");
//   console.log(sqINFO, (uint8_t *)&appKey, sizeof(appKey));
//   Serial.print("nwkSKey:");
//   console.log(sqINFO, (uint8_t *)&nwkSKey, sizeof(nwkSKey));
//   Serial.print("appSKey:");
//   console.log(sqINFO, (uint8_t *)&appSKey, sizeof(appSKey));
// }

// void SQ_LoRaWanClass::SQ_LoRaWan_generateDeveuiByChipID(bool simple)
// {
//   if (simple)
//   {
//     generate_lorawan_settings_by_chip_id();
//   }
//   else
//   {
//     generateDeveuiByChipID();
//   }
// }

// #endif