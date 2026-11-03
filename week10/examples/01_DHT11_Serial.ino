#include <DHT.h>
constexpr uint8_t DHT_PIN = 4;
DHT dht(DHT_PIN, DHT11);
void setup() { Serial.begin(115200); dht.begin(); Serial.println("DHT11 serial test"); }
void loop() {
  float h=dht.readHumidity(), t=dht.readTemperature();
  if (isnan(h)||isnan(t)) Serial.println("DHT11 read failed: check wiring / pull-up");
  else Serial.printf("Temperature: %.1f C | Humidity: %.1f %%\n",t,h);
  delay(2000);
}
