#include "bsp.h"
#include "siliqs_esp32.h"
#include "sensors/gps_measurement.h"

SemaphoreHandle_t serial1Mutex = xSemaphoreCreateMutex();
GPSMeasurement gps(GPS_RX_PIN, GPS_TX_PIN);
void setup()
{
  siliqs_esp32_setup();
  pinMode(GPS_VCTRL_PIN, OUTPUT);
  digitalWrite(GPS_VCTRL_PIN, LOW);

  gps.start(1000, &serial1Mutex);
}

void loop()
{
  static int c = 0;
  if (gps.gpsData.valid)
  {
    Serial.printf("Lat: %.2f Lon: %.2f Alt: %.2f\n", gps.gpsData.latitude, gps.gpsData.longitude, gps.gpsData.altitude);
    Serial.printf("%02d/%02d/%02dT%02d:%02d:%02d+%02d:%02d\n", gps.gpsData.time.yy, gps.gpsData.time.mm, gps.gpsData.time.dd, gps.gpsData.time.hr, gps.gpsData.time.min, gps.gpsData.time.sec, gps.gpsData.time.utcOffset_hours, gps.gpsData.time.utcOffset_minutes);
    c++;
  }

  if (c > 10)
  {
    gps.stop();
    digitalWrite(GPS_VCTRL_PIN, HIGH);
    Serial.println("GPS standby");
    c++;
  }
  if (c == 30)
  {
    digitalWrite(GPS_VCTRL_PIN, LOW);
    Serial.println("GPS restart");
    gps.start(1000, &serial1Mutex);
    c = 0;
  }
  delay(1000);
}
