/*
 * 실습 5-4  LED 레벨미터 + 경고음  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 3.3V / GND, Wiper → GPIO34
 *        LED1~6 (초록, 초록, 파랑, 파랑, 빨강, 빨강) : GPIO16, 17, 18, 19, 21, 22 → 330Ω → LED → GND
 *        수동 부저 (+) → GPIO23, (-) → GND
 *        GPIO13 ↔ 푸시버튼 ↔ GND   (경고음 ON/OFF)
 *
 * 핵심 : ADC(0~4095)를 7단계로 재양자화 + 히스테리시스 + 레벨 6 경고음
 *        ESP32 ADC 는 잡음이 커서 HYST 를 UNO 보다 크게 잡는다.
 */

const int POT_PIN = 34;
const int LED_PINS[6] = {16, 17, 18, 19, 21, 22};
const int BUZZER_PIN = 23;
const int BTN_PIN = 13;

const long ADC_RANGE = 4096;
const int  LEVELS = 7;
const int  HYST = 40;

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

int boundary(int k) { return (int)(k * ADC_RANGE / LEVELS); }
int upTh(int lv)    { return boundary(lv + 1) + HYST; }
int downTh(int lv)  { return boundary(lv) - HYST; }

void showLevel() {
  for (int i = 0; i < 6; i++) digitalWrite(LED_PINS[i], i < level ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 6; i++) pinMode(LED_PINS[i], OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  Serial.println("# 실습 5-4 LED 레벨미터 (ESP32)");
  Serial.print("# 경계값: ");
  for (int k = 1; k < LEVELS; k++) Serial.printf("%d ", boundary(k));
  Serial.printf(" HYST=%d\n", HYST);
}

void loop() {
  unsigned long now = millis();

  if (buttonPressed()) {
    alarmEnabled = !alarmEnabled;
    if (!alarmEnabled) noTone(BUZZER_PIN);
    Serial.printf("# 경고음 %s\n", alarmEnabled ? "ON" : "OFF");
  }

  if (now - lastSampleMs >= 20) {
    lastSampleMs = now;
    int adc = analogRead(POT_PIN);
    int old = level;
    while (level < LEVELS - 1 && adc >= upTh(level)) level++;
    while (level > 0 && adc < downTh(level)) level--;
    if (level != old) {
      showLevel();
      Serial.printf("Level=%d ADC=%d %s  다음 상승 ≥%d / 하강 <%d\n", level, adc,
                    level > old ? "(올라감)" : "(내려감)",
                    level < LEVELS - 1 ? upTh(level) : -1, level > 0 ? downTh(level) : -1);
    }
  }

  if (level == LEVELS - 1 && alarmEnabled && now - lastBeepMs >= 500) {
    lastBeepMs = now;
    tone(BUZZER_PIN, 1000, 100);
  }
}
