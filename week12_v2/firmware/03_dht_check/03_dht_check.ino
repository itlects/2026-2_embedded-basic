#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error "Select ESP32S3 Dev Module for this v2 sketch"
#endif

#include <DHT.h>
#include <math.h>
DHT dht(4, DHT11);
unsigned long lastSample = 0;
void setup() {
  Serial.begin(115200);
  delay(500);
  dht.begin();
  Serial.println("DHT11 check: GPIO4, 3.3V, common GND");
}
void loop() {
  if (millis() - lastSample < 2500) return;
  lastSample = millis();
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("FAIL: check DHT type, DATA, power and pull-up");
    return;
  }
  Serial.printf("T=%.1f C, H=%.1f %%\n", t, h);
}
