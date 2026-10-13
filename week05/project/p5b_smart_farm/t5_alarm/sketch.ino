/*
 * 스마트팜 T5 — 물 부족 경보 (빨강 LED 점멸 + 멜로디) 논블로킹 테스트
 * 토양 < 15 % 이면: 빨강 LED 0.5초 간격 점멸 + 4초마다 "솔-미-도" 경보음
 * BTN_MUTE(UNO D6 / ESP32 GPIO33) : 음소거 ON/OFF   ※ delay() 없이 동작해야 버튼이 바로 반응한다
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

const int DRY_ADC = ADC_MAX, WET_ADC = 0, ALARM_PCT = 15;
const int ALARM_NOTES[3] = {784, 659, 523};
const unsigned long NOTE_MS = 200, REPEAT_MS = 4000;
unsigned long alarmSeqMs = 0, lastBlinkMs = 0;
int alarmNote = -1;
bool muted = false, blinkOn = false;

bool btnStable = HIGH, btnLastRead = HIGH;
unsigned long btnChangedMs = 0;
bool mutePressed() {
  bool r = digitalRead(BTN_PINS[3]);
  if (r != btnLastRead) { btnLastRead = r; btnChangedMs = millis(); }
  if (millis() - btnChangedMs > 30 && r != btnStable) { btnStable = r; return btnStable == LOW; }
  return false;
}

int toPercent(int adc) { return constrain((int)map(adc, DRY_ADC, WET_ADC, 0, 100), 0, 100); }

void updateAlarmSound(unsigned long now, bool active) {
  if (!active || muted) {
    if (alarmNote >= 0) { noTone(BUZZER_PIN); alarmNote = -1; }
    return;
  }
  if (alarmNote < 0) {
    if (now - alarmSeqMs >= REPEAT_MS) {
      alarmSeqMs = now; alarmNote = 0;
      tone(BUZZER_PIN, ALARM_NOTES[0], NOTE_MS - 20);
    }
  } else if (now - alarmSeqMs >= (unsigned long)(alarmNote + 1) * NOTE_MS) {
    alarmNote++;
    if (alarmNote < 3) tone(BUZZER_PIN, ALARM_NOTES[alarmNote], NOTE_MS - 20);
    else alarmNote = -1;
  }
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  pinMode(ALARM_LED, OUTPUT);
  pinMode(BTN_PINS[3], INPUT_PULLUP);
  Serial.println(F("# T5 물 부족 경보 테스트 (노브를 건조 쪽 끝으로)"));
}

void loop() {
  unsigned long now = millis();
  if (mutePressed()) { muted = !muted; Serial.print(F("# 음소거 ")); Serial.println(muted ? F("ON") : F("OFF")); }
  if (now - lastBlinkMs >= 500) { lastBlinkMs = now; blinkOn = !blinkOn; }

  bool alarm = toPercent(analogRead(POT_PIN)) < ALARM_PCT;
  digitalWrite(ALARM_LED, (alarm && blinkOn) ? HIGH : LOW);
  updateAlarmSound(now, alarm);
}
