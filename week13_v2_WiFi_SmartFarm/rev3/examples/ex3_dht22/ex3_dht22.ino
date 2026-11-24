// 예제 3 : DHT22 온습도 센서 읽기
// 라이브러리 : DHT sensor library (Adafruit) + Adafruit Unified Sensor

#include <DHT.h>

const int DHT_PIN = 19;          // DHT22 DATA(SDA) -> GPIO19
DHT dht(DHT_PIN, DHT22);         // 센서 종류를 DHT22로 지정

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("DHT22 test start");
}

void loop() {
  delay(2000);                   // DHT22는 2초에 한 번만 측정 가능

  float h = dht.readHumidity();      // 상대습도(%)
  float t = dht.readTemperature();   // 온도(°C)

  if (isnan(h) || isnan(t)) {        // 읽기 실패 -> NaN(숫자 아님)
    Serial.println("DHT22 read failed - check wiring");
    return;
  }

  float hi = dht.computeHeatIndex(t, h, false);   // 체감온도(°C)
  Serial.printf("Temp=%.1f C  Humi=%.1f %%  HeatIndex=%.1f C\n", t, h, hi);
}
