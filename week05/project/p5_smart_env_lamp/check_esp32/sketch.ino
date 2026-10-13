/*
 * 종합과제 2단계 — 회로 점검 스케치 (ESP32 DevKitC V4)
 * I2C LCD : SDA GPIO21, SCL GPIO22, 주소 0x27 → 1행 "LCD OK", 2행 POT 값 표시
 * SENSOR_MODE : 0 DHT11(실물) / 1 DHT22(Wokwi)
 */
#define SENSOR_MODE 1

#include <DHT.h>
#if SENSOR_MODE == 0
  DHT dht(4, DHT11);
#else
  DHT dht(4, DHT22);
#endif

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);       // SDA 21, SCL 22, 안 나오면 0x3F

const int POT_PIN = 34;
const int LED_PINS[6] = {25, 26, 27, 16, 17, 18};
const char* LED_NAME[6] = {"상태 빨강(GPIO25)", "상태 초록(GPIO26)", "상태 파랑(GPIO27)",
                           "습도바1 파랑(GPIO16)", "습도바2 파랑(GPIO17)", "습도바3 빨강(GPIO18)"};
const int BUZZER_PIN = 23;
const int BTN_PINS[4] = {13, 14, 32, 33};
const char* BTN_NAME[4] = {"BTN_MODE(GPIO13)", "BTN_ALARM(GPIO14)", "BTN_REPORT(GPIO32)", "BTN_RESET(GPIO33)"};
bool lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastPrintMs = 0, lastSensorMs = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  for (int i = 0; i < 6; i++) pinMode(LED_PINS[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);
  dht.begin();
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("LCD OK          ");
  Serial.println("# [점검] LED 순서 확인");
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    Serial.printf("  ON  %s\n", LED_NAME[i]);
    delay(500);
    digitalWrite(LED_PINS[i], LOW);
  }
  Serial.println("# [점검] 부저 1kHz 0.3초");
  tone(BUZZER_PIN, 1000, 300);
  delay(400);
  Serial.println("# [점검] 가변저항을 돌리고 버튼을 하나씩 눌러 보세요");
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    bool r = digitalRead(BTN_PINS[i]);
    if (r == LOW && lastBtn[i] == HIGH) {
      Serial.printf("# 눌림: %s\n", BTN_NAME[i]);
      tone(BUZZER_PIN, 1500, 60);
    }
    lastBtn[i] = r;
  }

  if (now - lastPrintMs >= 500) {
    lastPrintMs = now;
    int adc = analogRead(POT_PIN);
    Serial.printf("POT=%d mV=%lu\n", adc, (unsigned long)analogReadMilliVolts(POT_PIN));
    char buf[17];
    snprintf(buf, sizeof(buf), "POT=%-4d        ", adc);
    lcd.setCursor(0, 1); lcd.print(buf);
  }

  if (now - lastSensorMs >= 2000) {
    lastSensorMs = now;
    float h = dht.readHumidity(), t = dht.readTemperature();
    Serial.printf("T=%.1fC H=%.0f%%%s\n", t, h, isnan(t) ? "  ← 센서 오류: 배선/라이브러리 확인" : "");
  }
}
