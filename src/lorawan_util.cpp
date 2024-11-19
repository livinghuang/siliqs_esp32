// #include "lorawan_util.h"
// #include "Arduino.h"
// #include "LoRaWan_APP.h"

// String bytes_to_string(uint8_t *data, int length)
// {
//   String result = "";
//   for (int i = 0; i < length; i++)
//   {
//     if (data[i] < 0x10)
//     {
//       result += "0";
//     }
//     result += String(data[i], HEX) + " ";
//   }
//   return result;
// }

// String devEuiString()
// {
//   return bytes_to_string((uint8_t *)&devEui, 8);
// }

// String appEuiString()
// {
//   return bytes_to_string((uint8_t *)&appEui, 8);
// }
// String appKeyString()
// {
//   return bytes_to_string((uint8_t *)&appKey, 16);
// }

// String nwkSKeyString()
// {
//   return bytes_to_string((uint8_t *)&nwkSKey, 16);
// }

// String appSKeyString()
// {
//   return bytes_to_string((uint8_t *)&appSKey, 16);
// }

// String devAddrString()
// {
//   String result = "";
//   char *data = (char *)&devAddr;
//   int length = 4;
//   for (int i = length - 1; i >= 0; i--)
//   {
//     if (data[i] < 0x10)
//     {
//       result += ("0");
//     }
//     result += String(data[i], HEX);
//     result += String(" ");
//   }
//   return result;
// }

// String loraWanRegionString()
// {
//   switch (loraWanRegion)
//   {
//   case LORAMAC_REGION_AS923:
//     return "AS923";
//   case LORAMAC_REGION_AU915:
//     return "AU915";
//   case LORAMAC_REGION_CN470:
//     return "CN470";
//   case LORAMAC_REGION_CN779:
//     return "CN779";
//   case LORAMAC_REGION_EU433:
//     return "EU433";
//   case LORAMAC_REGION_IN865:
//     return "IN865";
//   case LORAMAC_REGION_KR920:
//     return "KR920";
//   case LORAMAC_REGION_US915:
//     return "US915";
//   case LORAMAC_REGION_US915_HYBRID:
//     return "US915_HYBRID";
//   case LORAMAC_REGION_AS923_AS1:
//     return "AS923_AS1";
//   case LORAMAC_REGION_AS923_AS2:
//     return "AS923_AS2";
//   default:
//     return "Unknown";
//   }
// }
// String loraWanClassString()
// {
//   switch (loraWanClass)
//   {
//   case CLASS_A:
//     return "A";
//   case CLASS_B:
//     return "B";
//   case CLASS_C:
//     return "C";
//   default:
//     return "Unknown";
//   }
// }
// String appTxDutyCycleString()
// {
//   return String(appTxDutyCycle);
// }
// String otaaString()
// {
//   if (overTheAirActivation)
//   {
//     return "OTAA";
//   }
//   else
//   {
//     return "ABP";
//   }
// }
// String loraWanAdrString()
// {
//   if (loraWanAdr)
//   {
//     return "On";
//   }
//   else
//   {
//     return "Off";
//   }
// }
// String loraWanTxConfirmedString()
// {
//   if (isTxConfirmed)
//   {
//     return "On";
//   }
//   else
//   {
//     return "Off";
//   }
// }