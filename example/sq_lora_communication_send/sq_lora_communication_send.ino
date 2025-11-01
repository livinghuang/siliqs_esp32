#include "bsp.h"
#include "siliqs_esp32.h"

lora_params_settings params = {
    .DIO1 = LORA_DIO1,
    .BUSY = LORA_BUSY,
    .NRST = LORA_NRST,
    .MISO = LORA_MISO,
    .MOSI = LORA_MOSI,
    .SCK = LORA_SCK,
    .NSS = LORA_NSS,
    .FREQUENCY = 922.0,
    .BANDWIDTH = 125.0,
    .SF = 7,
    .CR = 5,
    .SYNC_WORD = 0x34,
    .OUTPUT_POWER = 22,
    .PREAMBLE_LENGTH = 8};

// Create an instance of LoRaService
LoRaService loraService(&params);
String message = "Hello";

void setup()
{
    siliqs_esp32_setup(SQ_INFO);
    // Initialize LoRa service
    if (!loraService.begin())
    {
        Serial.println(F("Failed to initialize LoRa!"));
        while (true)
        {
            delay(1000); // Stop further execution
        }
    }
}

void loop()
{
    loraService.sendMessage(message);
    Serial.println("Sending message: " + message);
    delay(1000); // Give some time for background tasks
}
