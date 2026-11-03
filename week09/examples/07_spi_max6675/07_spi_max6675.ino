// Week09 Example 7: SPI 열전대 온도 센서 MAX6675 — SPI 라이브러리로 직접 16비트 읽기
// ESP32: SCK=GPIO18, MISO(SO)=GPIO19, CS=GPIO4, VCC=3V3  /  UNO: SCK=D13, MISO=D12, CS=D10, VCC=5V
// MAX6675는 MOSI(입력)가 없다: 마스터는 클럭만 주고 데이터는 SO로만 나온다.
// 16비트 프레임: bit15=0(더미), bit14..3 = 온도(0.25°C 단위), bit2 = 1이면 열전대 미연결
#include <SPI.h>
#if defined(ESP32)
const int CS_PIN = 4;
#else
const int CS_PIN = 10;
#endif
SPISettings maxSettings(1000000, MSBFIRST, SPI_MODE0);   // 1MHz(최대 4.3MHz), 모드0

uint16_t readFrame() {
  SPI.beginTransaction(maxSettings);
  digitalWrite(CS_PIN, LOW);           // CS LOW = 이 장치 선택, 변환 중단·데이터 출력
  uint16_t v = SPI.transfer16(0x0000); // 16클럭 동안 MISO로 16비트 수신
  digitalWrite(CS_PIN, HIGH);          // CS HIGH = 선택 해제, 새 변환 시작(약 220ms)
  SPI.endTransaction();
  return v;
}

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
#if defined(ESP32)
  SPI.begin(18, 19, 23, CS_PIN);       // SCK, MISO, MOSI(미사용), SS
#else
  SPI.begin();
#endif
  delay(250);
  Serial.println(F("tempC"));
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last < 500) return;   // 변환 시간 ≥ 220ms
  last = millis();
  uint16_t f = readFrame();
  Serial.print(F("# raw=0x")); Serial.println(f, HEX);
  if (f == 0x0000 || f == 0xFFFF) { Serial.println(F("no sensor (check MISO/CS/VCC)")); return; }
  if (f & 0x0004)                 { Serial.println(F("thermocouple open")); return; }
  float tempC = (f >> 3) * 0.25f;
  Serial.println(tempC, 2);
}
