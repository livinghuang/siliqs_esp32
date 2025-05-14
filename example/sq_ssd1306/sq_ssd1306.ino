#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define pSSD1306_SCL 19
#define pSSD1306_SDA 18
#define pV_SSD1306_Ctrl 36 // 假設是 OLED 電源控制腳位

#define pVEXT 4            // 假設是 others 電源控制腳位 (其他I2C Device 共用 I2C BUS 要先把BUS 拉高以免被拉住)

#define OLED_RESET    -1  // 沒有使用 Reset 腳位的話設為 -1


void setup() {
  
  pinMode(pVEXT, OUTPUT);
  digitalWrite(pVEXT, LOW); // 開啟 others 電源控制腳位 (其他I2C Device 共用 I2C BUS 要先把BUS 拉高以免被拉住)
  delay(10);
  pinMode(pV_SSD1306_Ctrl, OUTPUT);
  digitalWrite(pV_SSD1306_Ctrl, LOW); // 開啟 OLED 電源 (視模組特性，可能要 HIGH)

  // 手動設定 I2C 腳位
  Wire.begin(pSSD1306_SDA, pSSD1306_SCL);
    delay(10);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  Serial.begin(115200);

  // 初始化 OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // 停止執行
  }

  display.clearDisplay();
  display.setTextSize(1);             // 文字大小
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);            // 設定起始座標
  display.println(F("Hello, OLED!"));
  display.display();                  // 實際顯示內容
}

void loop() {
  // 可加入其他畫面邏輯
}
