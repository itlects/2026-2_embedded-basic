// Week09 Example 6D: I2C 마스터(UNO) — 슬레이브(0x08, 예제 6E)에게 DHT11 값을 요청하고, 온도가 높으면 LED 켜기 명령
// 결선: A4(SDA)-A4, A5(SCL)-A5, GND-GND (같은 이름끼리, UART처럼 교차하지 않음)
#include <Wire.h>
const uint8_t SLAVE_ADDR = 0x08;
const int ALARM_TEMP = 28;                // 이 온도 이상이면 슬레이브 LED ON

void setup() {
  Serial.begin(115200);
  Wire.begin();                           // 주소 없이 = 마스터
  Serial.println(F("I2C MASTER: request 4 bytes from 0x08 every 2 s"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last < 2000) return;
  last = millis();

  // 1) 읽기: START + 0x08(R) → 슬레이브 onRequest가 4바이트 전송 → STOP
  uint8_t n = Wire.requestFrom(SLAVE_ADDR, (uint8_t)4);
  if (n != 4) { Serial.println(F("no answer from 0x08 (wiring / slave sketch?)")); return; }
  uint8_t st = Wire.read(), t = Wire.read(), h = Wire.read(), ck = Wire.read();
  if ((uint8_t)(st + t + h) != ck) { Serial.println(F("checksum error")); return; }
  if (st != 0) { Serial.println(F("slave: DHT11 read fail")); return; }

  // 2) 쓰기: START + 0x08(W) + 명령 1바이트 → ACK → STOP
  bool on = (int8_t)t >= ALARM_TEMP;
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(on ? 1 : 0);
  uint8_t err = Wire.endTransmission();   // 0 = ACK

  Serial.print(F("T=")); Serial.print((int8_t)t); Serial.print(F("C  H=")); Serial.print(h);
  Serial.print(F("%  -> LED ")); Serial.print(on ? F("ON") : F("OFF"));
  Serial.println(err == 0 ? F("  (ACK)") : F("  (NACK)"));
}
