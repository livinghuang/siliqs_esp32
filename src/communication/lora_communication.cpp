#include "bsp.h"
#ifdef USE_LORA
#include "lora_communication.h"

// 初始化静态成员变量
char LoraCommunication::txpacket[BUFFER_SIZE];
char LoraCommunication::rxpacket[BUFFER_SIZE];
bool LoraCommunication::lora_idle = true;
RadioEvents_t LoraCommunication::RadioEvents;

// 构造函数
LoraCommunication::LoraCommunication() {}

// 初始化LoRa设备
void LoraCommunication::begin()
{
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // 设置 Radio 事件回调，直接指向静态函数
  RadioEvents.TxDone = LoraCommunication::OnTxDone;
  RadioEvents.RxDone = LoraCommunication::OnRxDone;

  // 初始化 Radio 并设置参数
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void LoraCommunication::send(const char *data)
{
  if (lora_idle)
  {
    snprintf(txpacket, BUFFER_SIZE, "%s", data); // 设置数据包
    console.log(sqINFO, "Sending packet: " + String(txpacket) + ", length: " + String(strlen(txpacket)));
    Radio.Send((uint8_t *)txpacket, strlen(txpacket)); // 发送数据包
    lora_idle = false;
  }
  Radio.IrqProcess();
}

void LoraCommunication::receive(char *buffer, int length, int timeout)
{
  if (lora_idle)
  {
    console.log(sqINFO, "Switching to RX mode");
    Radio.Rx(0);
    lora_idle = false;
  }
  Radio.IrqProcess();
  strncpy(buffer, rxpacket, length); // 将接收到的数据复制到buffer
}

// 发送完成的回调函数
void LoraCommunication::OnTxDone()
{
  console.log(sqINFO, "TX done.");
  lora_idle = true;
}

// 接收完成的回调函数
void LoraCommunication::OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  memcpy(LoraCommunication::rxpacket, payload, size);
  LoraCommunication::rxpacket[size] = '\0'; // 终止接收到的数据包

  console.log(sqINFO, "Received packet: " + String(LoraCommunication::rxpacket) + ", RSSI: " + String(rssi) + ", Size: " + String(size));
  lora_idle = true;
}

#endif