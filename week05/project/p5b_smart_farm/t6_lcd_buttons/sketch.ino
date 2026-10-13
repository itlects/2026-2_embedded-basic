/*
 * 스마트팜 T6 — I2C LCD + 버튼 4개 테스트
 * LCD 1행: 토양 ADC 값, 2행: 마지막으로 누른 버튼 이름
 * 화면이 안 나오면: 뒷면 대비 가변저항 조절 → 주소 0x27 을 0x3F 로 변경
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if defined(ESP32)
const int POT_PIN = 34, DHT_PIN = 4;
const int GROW_R = 25, GROW_B = 26, PUMP_LED = 16, FAN_LED = 17, ALARM_LED = 18;
const int BUZZER_PIN = 23;
const int BTN_PINS[4] = {13, 14, 32, 33};        // MODE, PUMP, FAN, MUTE
const int ADC_MAX = 4095;
#else
const int POT_PIN = A0, DHT_PIN = 4;
const int GROW_R = 9, GROW_B = 10, PUMP_LED = 7, FAN_LED = 8, ALARM_LED = 13;
const int BUZZER_PIN = 11;
const int BTN_PINS[4] = {2, 3, 5, 6};
const int ADC_MAX = 1023;
#endif

LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* BTN_NAME[4] = {"MODE", "PUMP", "FAN", "MUTE"};
bool lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastLcdMs = 0;

void lcdLine(uint8_t row, const char* text) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", text);
  lcd.setCursor(0, row); lcd.print(buf);
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  for (int i = 0; i < 4; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);
  lcd.init(); lcd.backlight();
  lcdLine(0, "LCD OK");
  lcdLine(1, "Press a button");
  Serial.println(F("# T6 LCD + 버튼 테스트"));
}

void loop() {
  for (int i = 0; i < 4; i++) {
    bool r = digitalRead(BTN_PINS[i]);
    if (r == LOW && lastBtn[i] == HIGH) {
      char msg[17];
      snprintf(msg, sizeof(msg), "BTN: %s", BTN_NAME[i]);
      lcdLine(1, msg);
      Serial.print(F("# 눌림: ")); Serial.println(BTN_NAME[i]);
    }
    lastBtn[i] = r;
  }
  if (millis() - lastLcdMs >= 300) {
    lastLcdMs = millis();
    char l1[17];
    snprintf(l1, sizeof(l1), "SOIL ADC: %d", analogRead(POT_PIN));
    lcdLine(0, l1);
  }
}
