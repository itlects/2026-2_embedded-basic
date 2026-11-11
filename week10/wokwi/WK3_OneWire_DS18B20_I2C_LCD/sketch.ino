// =====================================================================
// Wokwi 워크북 C — 진짜 1-Wire(DS18B20 ×2) + I²C(LCD1602) + DHT 단일선 비교
//   GPIO13 : 1-Wire 버스(DQ) — DS18B20 2개 병렬, 4.7kΩ 풀업 → 3V3
//   GPIO21 : I²C SDA,  GPIO22 : I²C SCL — LCD1602(PCF8574 백팩, 주소 0x27)
//   GPIO4  : DHT22 DATA (DHT 고유 단일선 — 1-Wire도 I²C도 아님)
// 학습 포인트
//   ① 1-Wire는 64비트 ROM 주소로 한 선에 여러 장치를 구별한다(검색 → 주소 출력)
//   ② I²C는 7비트 주소로 장치를 부른다(스캐너로 0x27 발견)
//   ③ DHT는 주소가 없어 한 선에 센서 1개만 둔다
// 라이브러리(libraries.txt): OneWire, DallasTemperature, LiquidCrystal I2C,
//                            DHT sensor library for ESPx
// =====================================================================
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"

constexpr uint8_t ONE_WIRE_PIN = 13;
constexpr uint8_t DHT_PIN      = 4;
constexpr uint8_t SDA_PIN = 21, SCL_PIN = 22;
constexpr uint8_t LCD_ADDR = 0x27;

OneWire           oneWire(ONE_WIRE_PIN);
DallasTemperature ds(&oneWire);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
DHTesp            dht;

DeviceAddress addr[4];
uint8_t dsCount = 0;

void printRom(const uint8_t* a) {
  for (int i = 0; i < 8; i++) Serial.printf("%02X%s", a[i], i < 7 ? ":" : "");
}

// ---- I²C 스캐너: 주소 1~126에 START+주소를 보내 ACK가 오는지 확인 ----
void scanI2C() {
  Serial.println("[I2C] 버스 스캔(SDA=21, SCL=22)");
  int found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {          // 0 = ACK 받음
      Serial.printf("  장치 발견: 0x%02X\n", a);
      found++;
    }
  }
  if (!found) Serial.println("  장치 없음 → SDA/SCL/전원/주소 확인");
}

// ---- 1-Wire 검색: ROM 주소를 하나씩 찾아 CRC 검사 ----------------------
void scanOneWire() {
  Serial.println("[1-Wire] ROM 검색(GPIO13)");
  uint8_t a[8];
  oneWire.reset_search();
  while (oneWire.search(a) && dsCount < 4) {
    bool crcOk = OneWire::crc8(a, 7) == a[7];
    Serial.print("  ROM = "); printRom(a);
    Serial.printf("  family=0x%02X(%s) CRC %s\n", a[0],
                  a[0] == 0x28 ? "DS18B20" : "?", crcOk ? "OK" : "FAIL");
    memcpy(addr[dsCount++], a, 8);
  }
  Serial.printf("  센서 %u개 발견\n", dsCount);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  scanI2C();

  ds.begin();
  ds.setResolution(12);          // 12비트: 0.0625°C 단위, 변환 최대 750ms
  scanOneWire();

  dht.setup(DHT_PIN, DHTesp::DHT22);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("1-Wire + I2C");
  lcd.setCursor(0, 1); lcd.printf("DS18B20 x%u", dsCount);
  delay(1500);
}

void loop() {
  ds.requestTemperatures();                         // 버스 전체에 '변환 시작'(SKIP ROM)
  float t1 = dsCount > 0 ? ds.getTempC(addr[0]) : NAN;   // 주소로 개별 읽기(MATCH ROM)
  float t2 = dsCount > 1 ? ds.getTempC(addr[1]) : NAN;
  TempAndHumidity v = dht.getTempAndHumidity();

  Serial.printf("DS1=%.2fC  DS2=%.2fC  |  DHT22 T=%.1fC RH=%.1f%%\n",
                t1, t2, v.temperature, v.humidity);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.printf("A%4.1f B%4.1f C", t1, t2);
  lcd.setCursor(0, 1); lcd.printf("DHT%4.1fC %3.0f%%", v.temperature, v.humidity);
  delay(2000);
}
