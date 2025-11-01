#include "bsp.h"
#include "siliqs_esp32.h"
#include "esp_sleep.h"

lorawan_params_settings params = {
    .DIO1 = LORA_DIO1,
    .BUSY = LORA_BUSY,
    .NRST = LORA_NRST,
    .MISO = LORA_MISO,
    .MOSI = LORA_MOSI,
    .SCK = LORA_SCK,
    .NSS = LORA_NSS,

    .uplinkIntervalSeconds = 15, // 只作為參考；本例用 loop 控
    .ADR = true,
    .DR = 5,                 // 起始 DR，ADR 開啟後會被網路調整
    .DutyCycleFactor = 1250, // 1/1250 ≈ 0.08%，僅 EU 適用；AS923 可視需求 0
    .DwellTime = 400,        // 只對 US/AU 有 Dwell 規範；AS923 留 0 或依 RP 設
    .OTAA = true,
    .LORAWAN_1_1 = false,

    .JOINEUI = 0x0000000000000000ULL,
    .DEVEUI = 0x104ab88888888888ULL,
    .APPxKEY = {0xab, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
    .NWKxKEY = {0xab, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
    .DEVADDR = 0x55555555,
    // 你已選 LoRaWAN 1.0.x（LORAWAN_1_1=false），FNWKSINT/SNWKSINT 會被忽略
    .FNWKSINT = {0xff}, // 保留
    .SNWKSINT = {0xff}  // 保留
};

LoRaWanService lorawan(&params);

static uint32_t lastMillis = 0;

void setup()
{
  siliqs_esp32_setup(SQ_DEBUG);

  lorawan.setClass(LoRaWANClass::C);
  lorawan.begin();

  lorawan.startClassC();
}

void loop()
{
  uint8_t downBuffer[255];
  size_t downLen = 0;
  uint8_t downPort = 0;
  uint32_t now = millis();

  if (now - lastMillis >= 60000UL)
  {
    lastMillis = now;

    String updata = "hello world " + String(millis());
    static uint8_t fport = 10;

    bool isConfirmed = true;
    lorawan.set_battery_level(146);

    lorawan.send_and_receive(
        (const uint8_t *)updata.c_str(),
        updata.length(),
        fport,
        downBuffer,
        &downLen,
        &downPort,
        isConfirmed);
  }
  // 維持 Class C 常駐接收
  if (lorawan.pollClassC(downBuffer, &downLen, &downPort) == true)
  {
    Serial.print("Received from ");
    Serial.print(downPort);
    Serial.print(": ");
    for (int i = 0; i < downLen; i++)
    {
      Serial.print(downBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}