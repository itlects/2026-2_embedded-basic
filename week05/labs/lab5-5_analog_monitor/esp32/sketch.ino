/*
 * 실습 5-5  통합 Analog Monitor (3모드)  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 3.3V / GND, Wiper → GPIO34
 *        LED R → GPIO25, G → GPIO26, B → GPIO27   (각각 330Ω 직렬)
 *        수동 부저 (+) → GPIO23, (-) → GND
 *        GPIO13 ↔ 푸시버튼 ↔ GND   (모드 변경 LAMP → COLOR → THEREMIN)
 *
 * 모드 : LAMP / COLOR / THEREMIN  (UNO 버전과 동일)
 * 코어 : analogWrite()·tone() 모두 LEDC 사용 → Arduino-ESP32 코어 3.x 권장
 */

const int POT_PIN = 34;
const int LED_R = 25, LED_G = 26, LED_B = 27;
const int BUZZER_PIN = 23;
const int BTN_PIN = 13;
const int ADC_MAX = 4095;

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
  pct = (long)adc * 100 / ADC_MAX;
  int pwm = map(adc, 0, ADC_MAX, 0, 255);
  freq = 0;
  switch (mode) {
    case MODE_LAMP:
      r = g = b = pwm;
      break;
    case MODE_COLOR:
      if (pct < 50) { g = (int)(pct * 255.0 / 50 + 0.5); b = 255 - g; r = 0; }
      else          { r = (int)((pct - 50) * 255.0 / 50 + 0.5); g = 255 - r; b = 0; }
      break;
    case MODE_THEREMIN:
      freq = (200 + (long)adc * 1800 / ADC_MAX) / 10 * 10;
      r = 0; g = pwm; b = 0;
      break;
  }
}

void applyOutputs() {
  analogWrite(LED_R, r); analogWrite(LED_G, g); analogWrite(LED_B, b);
  if (mode == MODE_THEREMIN) {
    if (freq != lastFreq) { tone(BUZZER_PIN, freq); lastFreq = freq; }
  } else if (lastFreq != 0) {
    noTone(BUZZER_PIN); lastFreq = 0;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  Serial.println("# 실습 5-5 Analog Monitor (ESP32) - 버튼: LAMP → COLOR → THEREMIN");
}

void loop() {
  if (buttonPressed()) {
    mode = (Mode)((mode + 1) % 3);
    Serial.printf("# MODE -> %s\n", MODE_NAME[mode]);
  }

  if (millis() - lastSampleMs >= 20) {
    lastSampleMs = millis();
    adc = analogRead(POT_PIN);
    computeOutputs();
    applyOutputs();
  }

  if (millis() - lastPrintMs >= 200) {
    lastPrintMs = millis();
    float v = analogReadMilliVolts(POT_PIN) / 1000.0;
    Serial.printf("MODE=%s ADC=%d V=%.2f Level=%d%% RGB=%d/%d/%d Freq=%d\n",
                  MODE_NAME[mode], adc, v, pct, r, g, b, freq);
  }
}
