#include "bsp.h"
#ifdef USE_LED
#include "led.h"

class led led;
void test_led()
{
  led.begin(19, false, 200); // LED 接在 GPIO2
  led.blink(5);              // 閃爍 5 次

  while (1)
  {
    delay(10000);
    led.on(); // 長亮
    delay(2000);
    led.off(); // 關閉
    delay(2000);
    led.blink(3); // 再閃 3 次
  }
}
#endif