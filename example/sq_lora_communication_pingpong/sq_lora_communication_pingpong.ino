#include "bsp.h"
#include "siliqs_esp32.h"

lora_params_settings params = {
    .DIO1 = 14,
    .BUSY = 13,
    .NRST = 12,
    .MISO = 11,
    .MOSI = 10,
    .SCK = 9,
    .NSS = 8,
    .FREQUENCY = 922.0,
    .BANDWIDTH = 125.0,
    .SF = 7,
    .CR = 5,
    .SYNC_WORD = 0x34,
    .OUTPUT_POWER = 22,
    .PREAMBLE_LENGTH = 8};

// Create an instance of LoRaService
LoRaService loraService(&params);
String message = "Hello :0";

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

    Serial.println(F("LoRa initialized. Sending first message..."));
    loraService.sendMessage(message);
}

void loop()
{
    // Retrieve the last received message
    String receivedMessage = loraService.getReceivedMessage();
    if (!receivedMessage.isEmpty())
    {
        Serial.println("Received message: " + receivedMessage + " ,RSSI: " + String(loraService.radio.getRSSI()) + " ,SNR: " + String(loraService.radio.getSNR()));

        int delimiterIndex = receivedMessage.lastIndexOf(":");
        if (delimiterIndex != -1)
        {
            // Extract and increment the count
            String countStr = receivedMessage.substring(delimiterIndex + 1);
            int count = countStr.toInt();
            count++;
            message = "Hello :" + String(count);
            Serial.println("Sending message: " + message);

            // Delay to prevent immediate retransmission
            delay(1000);
            loraService.sendMessage(message);
        }
        else
        {
            Serial.println("Received malformed message, skipping.");
        }
    }

    delay(100); // Give some time for background tasks
}
