#if !defined(_RADIOLIB_H)
#define _RADIOLIB_H

/*!
  \mainpage RadioLib Documentation (Slimmed for SX1262 + LoRaWAN)

  Minimal RadioLib version for ESP32 projects.
  This build only supports:
  - SX1262 LoRa/FSK module
  - LoRaWAN (Class A, based on SX1262)
*/

#include "TypeDef.h"
#include "Module.h"

#include "Hal.h"
#if defined(RADIOLIB_BUILD_ARDUINO)
#include "hal/Arduino/ArduinoHal.h"
#endif

// warnings
#if RADIOLIB_GODMODE
#warning "God mode active, I hope it was intentional. Buckle up, lads."
#endif

#if RADIOLIB_DEBUG
#pragma message(RADIOLIB_INFO)
#endif

#if defined(RADIOLIB_UNKNOWN_PLATFORM)
#warning "RadioLib might not be compatible with this Arduino board - check supported platforms!"
#endif

#if defined(RADIOLIB_LOWEND_PLATFORM)
#warning "Low-end platform detected, stability issues are likely!"
#endif

// === SX1262 driver ===
#include "modules/SX126x/SX1262.h"

// === LoRaWAN protocol ===
#include "protocols/LoRaWAN/LoRaWAN.h"

// === Utilities (keep only what LoRaWAN/SX1262 need) ===
#include "utils/CRC.h"
#include "utils/Cryptography.h"

#endif // _RADIOLIB_H