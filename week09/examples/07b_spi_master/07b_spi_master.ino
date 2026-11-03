// Week09 Example 7B: SPI 보드 간 통신 — 마스터 (Arduino UNO 또는 ESP32)
// 1초마다 'A'~'Z' 한 글자를 슬레이브(UNO, 예제 7C)로 보내고, 같은 클럭 동안 슬레이브가 보낸 바이트를 동시에 받는다.
// SPI는 전이중: transfer()가 '보내면서 동시에 받는다'. 받은 값은 슬레이브가 '직전'에 준비해 둔 응답이다.
//
// 결선(UNO 마스터 ↔ UNO 슬레이브): D10(SS)-D10, D11(MOSI)-D11, D12(MISO)-D12, D13(SCK)-D13, GND-GND  ※ 같은 이름끼리
// 결선(ESP32 마스터 ↔ UNO 슬레이브): GPIO4→D10, GPIO23→D11, GPIO18→D13, D12→[분압 1k/2k]→GPIO19, GND-GND
#include <SPI.h>
#if defined(ESP32)
const int SS_PIN = 4;                 // GPIO5(기본 SS)는 부팅 스트랩 → GPIO4를 수동 CS로
#else
const int SS_PIN = 10;                // UNO: D10을 OUTPUT으로 두어야 마스터 모드가 유지된다
#endif
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);   // 1MHz, 모드0 (UNO 슬레이브는 최대 fosc/4 = 4MHz)

char sendChar = 'A';

void setup() {
  Serial.begin(115200);
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);         // 평소에는 선택 해제
#if defined(ESP32)
  SPI.begin(18, 19, 23, SS_PIN);      // SCK, MISO, MOSI, SS
#else
  SPI.begin();                        // SCK=D13, MISO=D12, MOSI=D11
#endif
  Serial.println(F("SPI MASTER: send A..Z every 1 s"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last < 1000) return;
  last = millis();

  SPI.beginTransaction(settings);
  digitalWrite(SS_PIN, LOW);                    // 1) 슬레이브 선택
  uint8_t reply = SPI.transfer(sendChar);       // 2) 8클럭: MOSI로 보내고 MISO로 동시에 받음
  digitalWrite(SS_PIN, HIGH);                   // 3) 선택 해제
  SPI.endTransaction();

  Serial.print(F("Send : ")); Serial.print(sendChar);
  Serial.print(F("   Recv : "));
  if (reply >= 32 && reply < 127) Serial.println((char)reply);
  else { Serial.print(F("0x")); Serial.println(reply, HEX); }   // 0xFF/0x00 = 슬레이브 없음·배선 확인

  sendChar = (sendChar == 'Z') ? 'A' : sendChar + 1;
}
