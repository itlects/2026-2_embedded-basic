/*
 * 스마트팜 워크북 — 완성 코드 (Arduino UNO / ESP32 공용)
 * ------------------------------------------------------------------
 * 가변저항 = 토양 수분 센서(아날로그) 대용, DHT = 온습도, LED = 펌프·팬·생장등·경보,
 * 수동 부저 = 물 부족 경보음, 버튼 4개, I2C LCD 16x2
 * 보드 종류에 따라 핀이 자동으로 선택된다 (#if defined(ESP32)).
 */
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHT_TYPE DHT22            // Wokwi: DHT22  /  실물 DHT11 사용 시 DHT11 로 변경

#if defined(ESP32)
const int POT_PIN = 34, DHT_PIN = 4;
const int GROW_R = 25, GROW_B = 26;              // 생장 LED (PWM)
const int PUMP_LED = 16, FAN_LED = 17, ALARM_LED = 18;
const int BUZZER_PIN = 23;
const int BTN_PINS[4] = {13, 14, 32, 33};        // MODE, PUMP, FAN, MUTE
const int ADC_MAX = 4095;
#else
const int POT_PIN = A0, DHT_PIN = 4;
const int GROW_R = 9, GROW_B = 10;
const int PUMP_LED = 7, FAN_LED = 8, ALARM_LED = 13;
const int BUZZER_PIN = 11;
const int BTN_PINS[4] = {2, 3, 5, 6};
const int ADC_MAX = 1023;
#endif

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- 기준값 ----------
const int DRY_ADC = ADC_MAX, WET_ADC = 0;         // 실제 토양센서: 마를수록 값이 커짐
const int PUMP_ON_PCT = 30, PUMP_OFF_PCT = 50;     // 펌프 히스테리시스
const int ALARM_PCT = 15;                          // 물 부족 경보
const float FAN_ON_T = 28.0, FAN_OFF_T = 27.0;     // 팬 히스테리시스 (온도)
const float FAN_ON_H = 70.0, FAN_OFF_H = 65.0;     // 팬 히스테리시스 (습도)
const unsigned long DAY_MS = 40000;                // 하루 = 40초로 축소
const unsigned long SUNRISE_END = 5000, SUNSET_START = 20000, SUNSET_END = 25000;

// ---------- 상태 ----------
enum Mode { MODE_AUTO, MODE_MANUAL };
Mode mode = MODE_AUTO;
bool pumpOn = false, fanOn = false, muted = false, blinkOn = false;
int soilAdc = 0, soilPct = 0, growPwm = 0;
float temperature = NAN, humidity = NAN;
unsigned long lastSoilMs = 0, lastDhtMs = 0, lastBlinkMs = 0, lastLcdMs = 0;

// ---------- 버튼 ----------
struct Button { int pin; bool stable; bool lastRead; unsigned long changedMs; };
Button buttons[4];
bool pressed(Button &b) {
  bool r = digitalRead(b.pin);
  if (r != b.lastRead) { b.lastRead = r; b.changedMs = millis(); }
  if (millis() - b.changedMs > 30 && r != b.stable) {
    b.stable = r;
    return b.stable == LOW;
  }
  return false;
}

// ---------- 계산 ----------
int toPercent(int adc) {                            // ADC → 토양 수분 %
  return constrain((int)map(adc, DRY_ADC, WET_ADC, 0, 100), 0, 100);
}

int growLevel(unsigned long now) {                  // 일출 → 낮 → 일몰 → 밤
  unsigned long p = now % DAY_MS;
  if (p < SUNRISE_END)  return map(p, 0, SUNRISE_END, 0, 255);
  if (p < SUNSET_START) return 255;
  if (p < SUNSET_END)   return map(p, SUNSET_START, SUNSET_END, 255, 0);
  return 0;
}

// ---------- 자동 제어 ----------
void autoPump() {
  if (!pumpOn && soilPct < PUMP_ON_PCT)        { pumpOn = true;  Serial.print(F("# 펌프 ON  (토양 ")); Serial.print(soilPct); Serial.println(F("%)")); }
  else if (pumpOn && soilPct >= PUMP_OFF_PCT)  { pumpOn = false; Serial.print(F("# 펌프 OFF (토양 ")); Serial.print(soilPct); Serial.println(F("%)")); }
}

void autoFan() {
  if (isnan(temperature) || isnan(humidity)) return;
  if (!fanOn && (temperature >= FAN_ON_T || humidity >= FAN_ON_H)) { fanOn = true;  Serial.println(F("# 팬 ON")); }
  else if (fanOn && temperature < FAN_OFF_T && humidity < FAN_OFF_H) { fanOn = false; Serial.println(F("# 팬 OFF")); }
}

// ---------- 경보음 (논블로킹) ----------
const int ALARM_NOTES[3] = {784, 659, 523};       // 솔-미-도
const unsigned long NOTE_MS = 200, REPEAT_MS = 4000;
unsigned long alarmSeqMs = 0;
int alarmNote = -1;                                // -1 = 쉬는 중

void updateAlarmSound(unsigned long now, bool active) {
  if (!active || muted) {
    if (alarmNote >= 0) { noTone(BUZZER_PIN); alarmNote = -1; }
    return;
  }
  if (alarmNote < 0) {
    if (now - alarmSeqMs >= REPEAT_MS) {           // 4초마다 한 소절
      alarmSeqMs = now; alarmNote = 0;
      tone(BUZZER_PIN, ALARM_NOTES[0], NOTE_MS - 20);
    }
  } else if (now - alarmSeqMs >= (unsigned long)(alarmNote + 1) * NOTE_MS) {
    alarmNote++;
    if (alarmNote < 3) tone(BUZZER_PIN, ALARM_NOTES[alarmNote], NOTE_MS - 20);
    else alarmNote = -1;
  }
}

// ---------- 출력 ----------
void report() {
  Serial.print(F("T=")); if (isnan(temperature)) Serial.print(F("--")); else Serial.print(temperature, 1);
  Serial.print(F("C H=")); if (isnan(humidity)) Serial.print(F("--")); else Serial.print(humidity, 0);
  Serial.print(F("% 토양=")); Serial.print(soilPct); Serial.print(F("%(ADC="));
  Serial.print(soilAdc); Serial.print(F(") | 펌프=")); Serial.print(pumpOn ? F("ON") : F("OFF"));
  Serial.print(F(" 팬=")); Serial.print(fanOn ? F("ON") : F("OFF"));
  Serial.print(F(" 조명=")); Serial.print(growPwm); Serial.print(growPwm > 0 ? F("(낮)") : F("(밤)"));
  Serial.print(F(" | 모드=")); Serial.print(mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
  Serial.print(F(" 음소거=")); Serial.println(muted ? F("ON") : F("OFF"));
}

void lcdLine(uint8_t row, const char* text) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", text);
  lcd.setCursor(0, row); lcd.print(buf);
}

void updateLcd() {
  char l1[17], l2[17], t[8];
  if (isnan(temperature) || isnan(humidity)) {
    snprintf(l1, sizeof(l1), "T--.- H--%% M%d%%", soilPct);
  } else {
    dtostrf(temperature, 4, 1, t);
    snprintf(l1, sizeof(l1), "T%s H%d%% M%d%%", t, (int)humidity, soilPct);
  }
  if (soilPct < ALARM_PCT && blinkOn) {
    snprintf(l2, sizeof(l2), "!! WATER LOW !!");
  } else {
    snprintf(l2, sizeof(l2), "%s P:%d F:%d %s", mode == MODE_AUTO ? "AUTO" : "MAN ",
             pumpOn, fanOn, growPwm > 0 ? "DAY" : "NGT");
  }
  lcdLine(0, l1); lcdLine(1, l2);
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  pinMode(GROW_R, OUTPUT); pinMode(GROW_B, OUTPUT);
  pinMode(PUMP_LED, OUTPUT); pinMode(FAN_LED, OUTPUT); pinMode(ALARM_LED, OUTPUT);
  for (int i = 0; i < 4; i++) {
    buttons[i] = {BTN_PINS[i], HIGH, HIGH, 0};
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }
  dht.begin();
  lcd.init(); lcd.backlight();
  lcdLine(0, "Smart Farm");
  lcdLine(1, "Week5 Workbook");
  Serial.println(F("# 스마트팜 완성 코드"));
  delay(1000);                                     // 시작 화면 (setup 에서만 사용)
}

void loop() {
  unsigned long now = millis();

  // 1) 버튼
  if (pressed(buttons[0])) {
    mode = (mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
    Serial.print(F("# 모드 -> ")); Serial.println(mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
  }
  if (pressed(buttons[1])) {
    if (mode == MODE_MANUAL) { pumpOn = !pumpOn; Serial.print(F("# 수동 펌프 ")); Serial.println(pumpOn ? F("ON") : F("OFF")); }
    else Serial.println(F("# AUTO 모드에서는 수동 펌프 버튼이 무시됩니다"));
  }
  if (pressed(buttons[2])) {
    if (mode == MODE_MANUAL) { fanOn = !fanOn; Serial.print(F("# 수동 팬 ")); Serial.println(fanOn ? F("ON") : F("OFF")); }
    else Serial.println(F("# AUTO 모드에서는 수동 팬 버튼이 무시됩니다"));
  }
  if (pressed(buttons[3])) {
    muted = !muted;
    Serial.print(F("# 음소거 ")); Serial.println(muted ? F("ON") : F("OFF"));
  }

  // 2) 토양 수분 (200 ms 샘플링)
  if (now - lastSoilMs >= 200) {
    lastSoilMs = now;
    soilAdc = analogRead(POT_PIN);
    soilPct = toPercent(soilAdc);
    if (mode == MODE_AUTO) autoPump();
  }

  // 3) 온습도 (2초)
  if (now - lastDhtMs >= 2000) {
    lastDhtMs = now;
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (mode == MODE_AUTO) autoFan();
    report();
  }

  // 4) 점멸 타이밍
  if (now - lastBlinkMs >= 500) { lastBlinkMs = now; blinkOn = !blinkOn; }

  // 5) 출력
  growPwm = growLevel(now);
  analogWrite(GROW_R, growPwm);
  analogWrite(GROW_B, growPwm);
  digitalWrite(PUMP_LED, pumpOn ? HIGH : LOW);
  digitalWrite(FAN_LED, fanOn ? HIGH : LOW);
  bool alarm = soilPct < ALARM_PCT;
  digitalWrite(ALARM_LED, (alarm && blinkOn) ? HIGH : LOW);
  updateAlarmSound(now, alarm);
  if (now - lastLcdMs >= 500) { lastLcdMs = now; updateLcd(); }
}
