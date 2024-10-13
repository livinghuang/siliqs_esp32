#include "bsp.h"
#include "siliqs_heltec_esp32.h"

/**                                                                                     \
 * @brief setup 函数，用于初始化系统                                          \
 *                                                                                      \
 * 该函数首先调用 siliqs_heltec_esp32_setup() 函数来初始化 ESP32 主板。 \
 */
void setup()
{
  siliqs_heltec_esp32_setup(SQ_INFO); // use SQ_INFO for show sqINFO message but not sqDEBUG message, use SQ_DEBUG for show sqDEBUG message, if you do not want to show any console message, use SQ_NONE or leave it blank
  console.log(sqINFO, "Serial Console initialized");
  Serial.begin(115200);

  // Write a test file
  fileSystem.writeFile("/testfile.txt", "Hello, SiliQ!");

  // List files before deleting
  fileSystem.listFiles();

  // Delete the test file
  fileSystem.deleteFile("/testfile.txt");

  // List files after deleting to confirm removal
  fileSystem.listFiles();
}

void loop()
{
  // 此處編寫你的代碼
  delay(1000);
  console.log(sqDEBUG, "this is a debug message, timestamp: %lu", millis());
}