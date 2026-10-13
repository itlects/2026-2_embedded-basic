/*
 * 종합과제 2단계 — 회로 점검 스케치 (Arduino UNO)
 * I2C LCD : SDA A4, SCL A5, 주소 0x27 → 1행 "LCD OK", 2행 POT 값 표시
 * SENSOR_MODE : 0 DHT11(실물) / 1 DHT22(Wokwi) / 2 TMP36(Tinkercad, A1)
 */
#define SENSOR_MODE 1

#if SENSOR_MODE == 0 || SENSOR_MODE == 1
  #include <DHT.h>
  #if SENSOR_MODE == 0
    DHT dht(4, DHT11);
  #else
    DHT dht(4, DHT22);
  #endif
#endif

#include <Wire.h>
#if SENSOR_MODE == 2
  #include <Adafruit_LiquidCrystal.h>
  Adafruit_LiquidCrystal lcd(0);          // Tinkercad I2C LCD (주소 0x20)
#else
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27, 16, 2);     // 안 나오면 0x3F
#endif

const int POT_PIN = A0;
const int LED_PINS[6] = {9, 6, 5, 7, 8, 13};
const char* LED_NAME[6] = {"상태 빨강(D9)", "상태 초록(D6)", "상태 파랑(D5)",
                           "습도바1 파랑(D7)", "습도바2 파랑(D8)", "습도바3 빨강(D13)"};
const int BUZZER_PIN = 11;
const int BTN_PINS[4] = {2, 3, 10, 12};
const char* BTN_NAME[4] = {"BTN_MODE(D2)", "BTN_ALARM(D3)", "BTN_REPORT(D10)", "BTN_RESET(D12)"};
bool lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastPrintMs = 0, lastSensorMs = 0;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 6; i++) pinMode(LED_PINS[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);
#if SENSOR_MODE != 2
  dht.begin();
#endif
#if SENSOR_MODE == 2
  lcd.begin(16, 2); lcd.setBacklight(1);
#else
  lcd.init(); lcd.backlight();
#endif
  lcd.setCursor(0, 0); lcd.print("LCD OK          ");
  Serial.println(F("# [점검] LED 순서 확인"));
  for (int i = 0; i < 6; i++) {           // setup 에서만 delay 사용
    digitalWrite(LED_PINS[i], HIGH);
    Serial.print(F("  ON  ")); Serial.println(LED_NAME[i]);
    delay(500);
    digitalWrite(LED_PINS[i], LOW);
  }
  Serial.println(F("# [점검] 부저 1kHz 0.3초"));
  tone(BUZZER_PIN, 1000, 300);
  delay(400);
  Serial.println(F("# [점검] 가변저항을 돌리고 버튼을 하나씩 눌러 보세요"));
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < 4; i++) {
    bool r = digitalRead(BTN_PINS[i]);
    if (r == LOW && lastBtn[i] == HIGH) {
      Serial.print(F("# 눌림: ")); Serial.println(BTN_NAME[i]);
      tone(BUZZER_PIN, 1500, 60);
    }
    lastBtn[i] = r;
  }

  if (now - lastPrintMs >= 500) {
    lastPrintMs = now;
    int adc = analogRead(POT_PIN);
    Serial.print(F("POT=")); Serial.print(adc);
    Serial.print(F(" V=")); Serial.println(adc * 5.0 / 1024.0, 2);
    char buf[17];
    snprintf(buf, sizeof(buf), "POT=%-4d        ", adc);
    lcd.setCursor(0, 1); lcd.print(buf);
  }

  if (now - lastSensorMs >= 2000) {
    lastSensorMs = now;
#if SENSOR_MODE == 2
    float t = (analogRead(A1) * 5.0 / 1024.0 - 0.5) * 100.0, h = 50.0;
#else
    float h = dht.readHumidity(), t = dht.readTemperature();
#endif
    Serial.print(F("T=")); Serial.print(t, 1);
    Serial.print(F("C H=")); Serial.print(h, 0);
    Serial.println(isnan(t) ? F("%  ← 센서 오류: 배선/라이브러리 확인") : F("%"));
  }
}
