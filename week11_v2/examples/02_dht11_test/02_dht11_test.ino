#include <DHT.h>
constexpr int DHT_PIN = 4;
DHT dht(DHT_PIN, DHT11);

void setup() {
  Serial.begin(115200);
  delay(1200);
  dht.begin();
  Serial.println("EX02 DHT11 read every 2s");
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("ERROR: DHT11 read failed - verify 3V3, GND, DATA.");
  } else {
    Serial.printf("temperature=%.1f C, humidity=%.1f %%\n", t, h);
  }
  delay(2000); // DHT11 must not be polled too frequently
}
