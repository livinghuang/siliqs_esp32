#include "bsp.h"
#include "siliqs_heltec_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
void setup()
{
  console.log(sqINFO, "Serial Console 已初始化");
}

void loop()
{
  // 此處編寫你的代碼
  delay(1000);
  console.log(sqDEBUG, "這是一條來自當前任務的調試消息，時間戳: %lu", millis());
}