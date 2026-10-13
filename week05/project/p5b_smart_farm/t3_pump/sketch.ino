/*
 * 스마트팜 T3 — 자동 급수 펌프(파랑 LED) 히스테리시스 테스트
 * 펌프 ON  : 토양 < 30 %
 * 펌프 OFF : 토양 ≥ 50 %   (30~49 % 구간에서는 이전 상태 유지)
 * 가변저항을 돌려 "흙이 마름 → 펌프 ON → 물이 차오름 → 펌프 OFF" 를 흉내 낸다.
 */
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

const int DRY_ADC = ADC_MAX, WET_ADC = 0;
const int PUMP_ON_PCT = 30, PUMP_OFF_PCT = 50;
bool pumpOn = false;
unsigned long lastSoilMs = 0;

int toPercent(int adc) { return constrain((int)map(adc, DRY_ADC, WET_ADC, 0, 100), 0, 100); }

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  pinMode(PUMP_LED, OUTPUT);
  Serial.println(F("# T3 펌프 히스테리시스 테스트"));
}

void loop() {
  if (millis() - lastSoilMs < 200) return;
  lastSoilMs = millis();
  int pct = toPercent(analogRead(POT_PIN));

  if (!pumpOn && pct < PUMP_ON_PCT)        { pumpOn = true;  Serial.print(F("# 펌프 ON  (토양 ")); Serial.print(pct); Serial.println(F("%)")); }
  else if (pumpOn && pct >= PUMP_OFF_PCT)  { pumpOn = false; Serial.print(F("# 펌프 OFF (토양 ")); Serial.print(pct); Serial.println(F("%)")); }
  digitalWrite(PUMP_LED, pumpOn ? HIGH : LOW);
}
