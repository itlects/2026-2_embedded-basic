/*
 * 실습 5-5  통합 Analog Monitor (3모드)  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 5V / GND, Wiper → A0
 *        LED R → D9, G → D6, B → D5   (각각 330Ω 직렬, PWM 핀)
 *        수동 부저 (+) → D11, (-) → GND
 *        D2 ↔ 푸시버튼 ↔ GND   (모드 변경 LAMP → COLOR → THEREMIN)
 *
 * 모드 : LAMP     — 3색 동일 밝기 무드등 (ADC → PWM 0~255)
 *        COLOR    — 파랑 → 초록 → 빨강 색상 게이지
 *                   0~50 %: B↓ G↑ / 50~100 %: G↓ R↑
 *        THEREMIN — 가변저항으로 음 높이 조절 (ADC → 200~2000 Hz, 10 Hz 단위)
 *
 * 출력 : MODE=COLOR ADC=612 V=2.99 Level=59% RGB=46/209/0 Freq=0
 *
 * 이 회로는 5주차 종합과제의 기본 골격이다 (DHT11, 습도바 LED, 버튼 3개를 추가).
 */

const int POT_PIN = A0;
const int LED_R = 9, LED_G = 6, LED_B = 5;
const int BUZZER_PIN = 11;
const int BTN_PIN = 2;
const int ADC_MAX = 1023;
const float VREF = 5.0;

enum Mode { MODE_LAMP, MODE_COLOR, MODE_THEREMIN };
const char* MODE_NAME[] = {"LAMP", "COLOR", "THEREMIN"};
Mode mode = MODE_LAMP;

int adc = 0, pct = 0, r = 0, g = 0, b = 0, freq = 0, lastFreq = 0;
unsigned long lastSampleMs = 0, lastPrintMs = 0;

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

void computeOutputs() {
  pct = (long)adc * 100 / ADC_MAX;                      // 0 ~ 100 %
  int pwm = map(adc, 0, ADC_MAX, 0, 255);
  freq = 0;
  switch (mode) {
    case MODE_LAMP:
      r = g = b = pwm;
      break;
    case MODE_COLOR:
      if (pct < 50) {                                   // 파랑 → 초록
        g = (int)(pct * 255.0 / 50 + 0.5); b = 255 - g; r = 0;
      } else {                                          // 초록 → 빨강
        r = (int)((pct - 50) * 255.0 / 50 + 0.5); g = 255 - r; b = 0;
      }
      break;
    case MODE_THEREMIN:
      freq = (200 + (long)adc * 1800 / ADC_MAX) / 10 * 10;   // 10 Hz 단위로 양자화
      r = 0; g = pwm; b = 0;                            // 음 높이를 초록 밝기로도 표시
      break;
  }
}

void applyOutputs() {
  analogWrite(LED_R, r); analogWrite(LED_G, g); analogWrite(LED_B, b);
  if (mode == MODE_THEREMIN) {
    if (freq != lastFreq) { tone(BUZZER_PIN, freq); lastFreq = freq; }   // 바뀔 때만 갱신
  } else if (lastFreq != 0) {
    noTone(BUZZER_PIN); lastFreq = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("# 실습 5-5 Analog Monitor (UNO) - 버튼: LAMP → COLOR → THEREMIN"));
}

void loop() {
  if (buttonPressed()) {
    mode = (Mode)((mode + 1) % 3);
    Serial.print(F("# MODE -> ")); Serial.println(MODE_NAME[mode]);
  }

  if (millis() - lastSampleMs >= 20) {                  // 50 Hz 샘플링
    lastSampleMs = millis();
    adc = analogRead(POT_PIN);
    computeOutputs();
    applyOutputs();
  }

  if (millis() - lastPrintMs >= 200) {
    lastPrintMs = millis();
    Serial.print(F("MODE=")); Serial.print(MODE_NAME[mode]);
    Serial.print(F(" ADC=")); Serial.print(adc);
    Serial.print(F(" V=")); Serial.print(adc * VREF / 1024.0, 2);
    Serial.print(F(" Level=")); Serial.print(pct);
    Serial.print(F("% RGB=")); Serial.print(r); Serial.print('/'); Serial.print(g); Serial.print('/'); Serial.print(b);
    Serial.print(F(" Freq=")); Serial.println(freq);
  }
}
