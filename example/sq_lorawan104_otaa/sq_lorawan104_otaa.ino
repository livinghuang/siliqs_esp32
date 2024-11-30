#include "bsp.h"
#include "siliqs_heltec_esp32.h"
#include "esp_sleep.h"
/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
#define RADIOLIB_DEBUG
#define LORA_DIO1 3
#define LORA_BUSY 4
#define LORA_NRST 5
#define LORA_MISO 6
#define LORA_MOSI 7
#define LORA_NSS 8
#define LORA_SCK 10

lorawan_params_settings params = {
    .DIO1 = LORA_DIO1,                                                                                            // DIO1
    .BUSY = LORA_BUSY,                                                                                            // BUSY
    .NRST = LORA_NRST,                                                                                            // reset
    .MISO = LORA_MISO,                                                                                            // MISO
    .MOSI = LORA_MOSI,                                                                                            // MOSI
    .SCK = LORA_SCK,                                                                                              // SCK
    .NSS = LORA_NSS,                                                                                              // NSS
    .uplinkIntervalSeconds = 15,                                                                                  // Unit: second, upline interval time
    .ADR = true,                                                                                                  // use ADR or not
    .DR = 5,                                                                                                      // Data Rate when start, if ADR is true, this will be tuned automatically
    .DutyCycleFactor = 1250,                                                                                      // Duty Cycle = 1 / (DutyCycleFactor) , if 0, disable. In EU law, Duty Cycle should under 1%
    .DwellTime = 400,                                                                                             // Unit: ms, Dwell Time to limit signal airtime in single channel, In US/AU law,Dwell Time under 400ms
    .OTAA = true,                                                                                                 // OTAA or ABP
    .LORAWAN_1_1 = false,                                                                                         // LORAWAN 1.1 or 1.0
    .JOINEUI = 0x0000000000000000,                                                                                // Join EUI
    .DEVEUI = 0x104aa88888888888,                                                                                 // DEVEUI, if OTAA, DEVEUI will used. if ABP, DEVEUI will be ignored
    .APPxKEY = {0xaa, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},  // if OTAA, APPxKEY = APPKEY, if ABP, APPxKEY = APPSKEY
    .NWKxKEY = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},  // if OTAA, NWKxKEY = NULL use lorawan v1.0.x, if ABP, APPxKEY = NWKSKEY
    .DEVADDR = 0xffffffff,                                                                                        // if ABP, DEVADDR, if OTAA, DEVADDR will be ignored
    .FNWKSINT = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FNWKSINT, if lorawan v1.0.x, set as NULL, if lorawan v1.1.x
    .SNWKSINT = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}  // SNWKSINT, if lorawan v1.0.x, set as NULL, if lorawan v1.1.x
};
LoRaWanService lorawan(&params);

void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO);
  lorawan.begin();
}

void loop()
{
  uint8_t updata[] = "hello world";
  static uint8_t fport = 10;
  uint8_t downbuffer[255];
  size_t downlen = 0;
  bool isConfirmed = true;
  lorawan.set_battery_level(146);
  lorawan.send_and_receive(updata, sizeof(updata), fport, downbuffer, &downlen, isConfirmed);

  if (downlen > 0)
  {
    Serial.print("Downlink: ");
    for (int i = 0; i < downlen; i++)
    {
      if (downbuffer[i] < 0x10)
      {
        Serial.print("0");
      }
      Serial.print(downbuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  lorawan.sleep(false); // sleep interval time base on params.uplinkIntervalSeconds, if you use TTN law,set it true and then it will calculate minimum duty cycle delay
}
