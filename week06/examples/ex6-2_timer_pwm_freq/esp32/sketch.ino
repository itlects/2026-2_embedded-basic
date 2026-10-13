/*
 * 예제 6-2  LEDC 타이머로 PWM 주파수·해상도·듀티 직접 설정  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : GPIO25 → 330Ω → LED(+) → LED(-) → GND
 *        가변저항: 양 끝 3.3V / GND, 가운데(Wiper) → GPIO34   (5V 연결 금지)
 *        GPIO13 ↔ 푸시버튼 ↔ GND                            (주파수 변경)
 *
 * LEDC(LED Control) 주변장치 = ESP32 의 PWM 전용 하드웨어 타이머
 *   - 카운터 클록  f_clk = 80 MHz(APB) / 분주비(div)
 *   - PWM 주파수   f_pwm = 80 MHz / (div × 2^res)   (res = 해상도 bit)
 *   - 분주비는 1 ~ 1023 범위 → 주파수를 낮추려면 해상도를 높여야 하고,
 *     주파수를 높이려면 해상도를 낮춰야 한다 (주파수 ↔ 해상도 트레이드오프)
 *
 * API (Arduino-ESP32 코어 3.x)
 *   ledcAttach(pin, freq, res)          : 핀에 PWM 채널 연결
 *   ledcChangeFrequency(pin, freq, res) : 주파수·해상도 변경
 *   ledcWrite(pin, duty)                : 듀티 설정 (0 ~ 2^res - 1)
 */

const int LED_PIN = 25;
const int POT_PIN = 34;         // ADC1 (입력 전용)
const int BTN_PIN = 13;

struct PwmSetting { uint32_t freq; uint8_t res; };
// 각 주파수에서 분주비가 1~1023 범위에 들어오도록 해상도를 고른 표
const PwmSetting SETTINGS[] = {
  {1,    17},   // 80e6 / (2^17 × 1)   → div ≈ 610
  {4,    15},   // 80e6 / (2^15 × 4)   → div ≈ 610
  {20,   13},   // 80e6 / (2^13 × 20)  → div ≈ 488
  {100,  10},   // 80e6 / (2^10 × 100) → div ≈ 781
  {1000, 10},   // 80e6 / (2^10 × 1000)→ div ≈ 78
};
const int SETTING_COUNT = sizeof(SETTINGS) / sizeof(SETTINGS[0]);
int setNo = 0;

unsigned long lastUpdateMs = 0, lastPrintMs = 0;
uint32_t dutyRaw = 0;

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

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);                         // 0 ~ 4095

  if (!ledcAttach(LED_PIN, SETTINGS[setNo].freq, SETTINGS[setNo].res)) {
    Serial.println("ledcAttach failed - check core 3.x");
  }
  Serial.println("Ex6-2 LEDC PWM (ESP32)");
}

void loop() {
  if (buttonPressed()) {
    setNo = (setNo + 1) % SETTING_COUNT;
    uint32_t real = ledcChangeFrequency(LED_PIN, SETTINGS[setNo].freq,
                                        SETTINGS[setNo].res);
    if (real == 0) Serial.println("frequency change failed");
  }

  // 20 ms 마다 가변저항 → 듀티
  if (millis() - lastUpdateMs >= 20) {
    lastUpdateMs = millis();
    int adc = analogRead(POT_PIN);                  // 0 ~ 4095
    uint32_t maxDuty = (1UL << SETTINGS[setNo].res) - 1;
    dutyRaw = (uint32_t)((uint64_t)maxDuty * adc / 4095);
    ledcWrite(LED_PIN, dutyRaw);
  }

  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    uint32_t maxDuty = (1UL << SETTINGS[setNo].res) - 1;
    Serial.printf("f=%luHz  res=%ubit  duty=%lu/%lu  Duty=%.1f%%\n",
                  (unsigned long)SETTINGS[setNo].freq, SETTINGS[setNo].res,
                  (unsigned long)dutyRaw, (unsigned long)maxDuty,
                  dutyRaw * 100.0 / maxDuty);
  }
}
