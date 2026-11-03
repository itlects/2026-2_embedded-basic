// Week09 Example 6: I2C 조도 센서 BH1750(GY-302) — 라이브러리 없이 Wire로 직접 통신
// ESP32: SDA=GPIO21, SCL=GPIO22, VCC=3V3  /  UNO: SDA=A4, SCL=A5, VCC=5V(모듈 레귤레이터) 또는 3.3V
// 프로토콜: [START][주소 0x23+W][명령 0x10][STOP] → 180ms 대기 → [START][0x23+R][상위바이트][하위바이트][STOP]
#include <Wire.h>
const uint8_t BH1750_ADDR = 0x23;        // ADDR 핀 = GND(또는 미연결)
const uint8_t CMD_POWER_ON = 0x01;
const uint8_t CMD_CONT_HRES = 0x10;      // 연속 측정, 1 lx 분해능, 약 120~180ms

bool sendCommand(uint8_t cmd) {
  Wire.beginTransmission(BH1750_ADDR);
  Wire.write(cmd);
  return Wire.endTransmission() == 0;    // 0 = 센서가 ACK
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  Wire.begin(21, 22);
#else
  Wire.begin();
#endif
  Wire.setClock(100000);                 // Standard mode 100kHz
  if (!sendCommand(CMD_POWER_ON) || !sendCommand(CMD_CONT_HRES)) {
    Serial.println(F("BH1750 no ACK - check wiring/address (run 06b scanner)"));
  }
  delay(180);
  Serial.println(F("lux"));              // 시리얼 플로터용 라벨
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last < 500) return;
  last = millis();
  if (Wire.requestFrom(BH1750_ADDR, (uint8_t)2) == 2) {
    uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();   // MSB 먼저
    float lux = raw / 1.2f;                                       // 데이터시트 환산식
    Serial.println(lux, 1);
  } else {
    Serial.println(F("read error"));
  }
}
