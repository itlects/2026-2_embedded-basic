// Week09 Example 6B: I2C 스캐너 — 버스에 연결된 장치의 7비트 주소를 찾는다
// ESP32: SDA=GPIO21, SCL=GPIO22  /  UNO: SDA=A4, SCL=A5
#include <Wire.h>
void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  Wire.begin(21, 22);
#else
  Wire.begin();
#endif
}
void loop() {
  int found = 0;
  Serial.println(F("Scanning I2C 0x08~0x77 ..."));
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);           // START + 주소 + W
    uint8_t err = Wire.endTransmission();   // 0 = ACK 받음
    if (err == 0) {
      Serial.print(F("  found 0x")); if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX); found++;
    }
  }
  Serial.print(F("devices: ")); Serial.println(found);
  Serial.println(F("(BH1750: 0x23 (ADDR=GND) or 0x5C (ADDR=VCC))"));
  delay(3000);
}
