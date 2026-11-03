// Week09 Example 8: 1-Wire 온도 센서 DS18B20 — OneWire 라이브러리로 명령을 직접 보낸다
// 필요 라이브러리: "OneWire" (Paul Stoffregen) — 라이브러리 매니저에서 설치
// ESP32: DQ=GPIO13, VDD=3V3  /  UNO: DQ=D2, VDD=5V   ※ DQ–VDD 사이 4.7kΩ 풀업 필수
// 순서: RESET → (Presence 확인) → ROM 명령 → 기능 명령 → 데이터
#include <OneWire.h>
#if defined(ESP32)
const int ONE_WIRE_PIN = 13;
#else
const int ONE_WIRE_PIN = 2;
#endif
OneWire ow(ONE_WIRE_PIN);
uint8_t addr[8];
bool haveSensor = false;

void printRom(const uint8_t *a) {
  for (int i = 0; i < 8; i++) { if (a[i] < 16) Serial.print('0'); Serial.print(a[i], HEX); Serial.print(i < 7 ? ':' : '\n'); }
}

void setup() {
  Serial.begin(115200);
  ow.reset_search();
  while (ow.search(addr)) {                 // SEARCH ROM(0xF0): 버스의 모든 장치 64비트 ID 찾기
    Serial.print(F("ROM "));
    printRom(addr);
    if (OneWire::crc8(addr, 7) != addr[7]) { Serial.println(F("  CRC error")); continue; }
    if (addr[0] == 0x28) { haveSensor = true; break; }   // 패밀리 코드 0x28 = DS18B20
  }
  if (!haveSensor) Serial.println(F("No DS18B20 (check 4.7k pull-up, DQ pin)"));
  Serial.println(F("tempC"));
}

void loop() {
  if (!haveSensor) return;
  // 1) 변환 시작: RESET → MATCH ROM(0x55)+ID → CONVERT T(0x44)
  ow.reset(); ow.select(addr); ow.write(0x44);
  delay(750);                               // 12비트 변환 최대 750ms
  // 2) 결과 읽기: RESET → MATCH ROM → READ SCRATCHPAD(0xBE) → 9바이트
  if (!ow.reset()) { Serial.println(F("no presence pulse")); return; }
  ow.select(addr); ow.write(0xBE);
  uint8_t d[9];
  for (int i = 0; i < 9; i++) d[i] = ow.read();
  if (OneWire::crc8(d, 8) != d[8]) { Serial.println(F("scratchpad CRC error")); return; }
  int16_t raw = (int16_t)((d[1] << 8) | d[0]);   // 1/16 °C 단위
  Serial.println(raw / 16.0f, 2);
}
