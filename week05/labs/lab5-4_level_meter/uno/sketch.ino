/*
 * 실습 5-4  LED 레벨미터 + 경고음  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 5V / GND, Wiper → A0
 *        LED1~6 (초록, 초록, 파랑, 파랑, 빨강, 빨강) : D3, D4, D5, D6, D7, D8 → 330Ω → LED → GND
 *        수동 부저 (+) → D11, (-) → GND
 *        D2 ↔ 푸시버튼 ↔ GND   (경고음 ON/OFF)
 *
 * 핵심 1 : 재양자화 — ADC(0~1023)를 7단계(레벨 0~6)로 다시 나눈다.  경계값 = k × 1024 / 7
 * 핵심 2 : 히스테리시스 — 경계 근처에서 레벨이 떨리는 것을 막기 위해
 *          올라갈 때는 경계 + HYST, 내려갈 때는 경계 - HYST 를 기준으로 쓴다.
 *          HYST = 0 으로 바꿔 경계 근처에서 LED 가 떨리는 것을 비교해 본다.
 * 핵심 3 : 레벨 6 이면 0.5초마다 tone(1000 Hz, 100 ms) 경고음 (delay 없이)
 *
 * 참고 : tone() 은 Timer2 를 쓰므로 D3·D11 은 PWM 불가 → D3 LED 는 디지털 ON/OFF 로만 사용
 */

const int POT_PIN = A0;
const int LED_PINS[6] = {3, 4, 5, 6, 7, 8};
const int BUZZER_PIN = 11;
const int BTN_PIN = 2;

const long ADC_RANGE = 1024;          // 0 ~ 1023
const int  LEVELS = 7;                // 레벨 0 ~ 6
const int  HYST = 10;                 // 히스테리시스 폭 (0 으로 바꿔 비교)

int level = 0;
bool alarmEnabled = true;
unsigned long lastSampleMs = 0, lastBeepMs = 0;

const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH, stableState = HIGH;
unsigned long lastChangeMs = 0;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) { lastChangeMs = millis(); lastReading = reading; }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

int boundary(int k) { return (int)(k * ADC_RANGE / LEVELS); }   // k = 1..6
int upTh(int lv)    { return boundary(lv + 1) + HYST; }         // lv → lv+1
int downTh(int lv)  { return boundary(lv) - HYST; }             // lv → lv-1

void showLevel() {
  for (int i = 0; i < 6; i++) digitalWrite(LED_PINS[i], i < level ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 6; i++) pinMode(LED_PINS[i], OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("# 실습 5-4 LED 레벨미터 (UNO)"));
  Serial.print(F("# 경계값: "));
  for (int k = 1; k < LEVELS; k++) { Serial.print(boundary(k)); Serial.print(' '); }
  Serial.print(F(" HYST=")); Serial.println(HYST);
}

void loop() {
  unsigned long now = millis();

  if (buttonPressed()) {
    alarmEnabled = !alarmEnabled;
    if (!alarmEnabled) noTone(BUZZER_PIN);
    Serial.print(F("# 경고음 ")); Serial.println(alarmEnabled ? F("ON") : F("OFF"));
  }

  if (now - lastSampleMs >= 20) {                       // 50 Hz 샘플링
    lastSampleMs = now;
    int adc = analogRead(POT_PIN);
    int old = level;
    while (level < LEVELS - 1 && adc >= upTh(level)) level++;
    while (level > 0 && adc < downTh(level)) level--;
    if (level != old) {
      showLevel();
      Serial.print(F("Level=")); Serial.print(level);
      Serial.print(F(" ADC=")); Serial.print(adc);
      Serial.print(level > old ? F(" (올라감)") : F(" (내려감)"));
      Serial.print(F("  다음 상승 ≥")); Serial.print(level < LEVELS - 1 ? upTh(level) : -1);
      Serial.print(F(" / 하강 <")); Serial.println(level > 0 ? downTh(level) : -1);
    }
  }

  if (level == LEVELS - 1 && alarmEnabled && now - lastBeepMs >= 500) {
    lastBeepMs = now;
    tone(BUZZER_PIN, 1000, 100);                        // 논블로킹 경고음
  }
}
