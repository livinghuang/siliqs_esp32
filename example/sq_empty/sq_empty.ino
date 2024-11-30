#include "bsp.h"
#include "siliqs_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
void setup()
{
  siliqs_esp32_setup(SQ_INFO); // use SQ_INFO for show sqINFO message but not sqDEBUG message, use SQ_DEBUG for show sqDEBUG message, if you do not want to show any console message, use SQ_NONE or leave it blank
  console.log(sqINFO, "Serial Console initialized");
}

void loop()
{
  // 此處編寫你的代碼
  delay(1000);
  console.log(sqDEBUG, "this is a debug message, timestamp: %lu", millis());
}