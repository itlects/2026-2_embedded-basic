/*
 * 스마트팜 T1 — 토양 수분 센서(가변저항) ADC → % 변환 테스트
 * 확인: 노브를 돌리면 ADC 와 토양 % 가 바뀐다. 오른쪽 끝(ADC 최대)=0%(건조), 왼쪽 끝=100%(젖음)
 * 실제 토양 수분 센서도 아날로그 전압을 출력하며, 마를수록 값이 커지는 제품이 많다.
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

const int DRY_ADC = ADC_MAX, WET_ADC = 0;      // 보정값: 완전히 마른 값 / 완전히 젖은 값
unsigned long lastSoilMs = 0;

int toPercent(int adc) {
  return constrain((int)map(adc, DRY_ADC, WET_ADC, 0, 100), 0, 100);
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  Serial.println(F("# T1 토양 수분 ADC 테스트"));
}

void loop() {
  if (millis() - lastSoilMs >= 200) {             // 200 ms 샘플링 (5 Hz)
    lastSoilMs = millis();
    int adc = analogRead(POT_PIN);
    Serial.print(F("ADC=")); Serial.print(adc);
    Serial.print(F(" 토양=")); Serial.print(toPercent(adc)); Serial.println(F("%"));
  }
}
