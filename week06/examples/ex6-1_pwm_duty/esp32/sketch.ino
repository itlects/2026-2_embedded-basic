/*
 * 예제 6-1  PWM 듀티비(Duty Cycle)와 LED 밝기  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : GPIO25 → 330Ω → LED(+, 긴 다리) → LED(-, 짧은 다리) → GND
 *        GPIO13 ↔ 푸시버튼 ↔ GND   (INPUT_PULLUP: 누르면 LOW)
 *
 * 동작 : 버튼을 누를 때마다 PWM 값이 0 → 32 → 64 → 128 → 192 → 255 로 바뀐다.
 *
 * 핵심 : ESP32 의 analogWrite() 는 내부적으로 LEDC(LED Control) 하드웨어
 *        타이머를 사용한다. Arduino-ESP32 코어 3.x 기본값은 1 kHz, 8-bit 이다.
 *        (코어 3.x 이상 필요: 보드 매니저 "esp32 by Espressif Systems")
 */

const int LED_PIN = 25;
const int BTN_PIN = 13;         // GPIO34~39 는 입력 전용·풀업 없음 → 버튼에 사용 금지

const int LEVELS[] = {0, 32, 64, 128, 192, 255};
const int LEVEL_COUNT = sizeof(LEVELS) / sizeof(LEVELS[0]);

const float PWM_FREQ_HZ = 1000.0;              // analogWrite 기본 주파수
const float PERIOD_MS   = 1000.0 / PWM_FREQ_HZ;

int levelNo = 0;

const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH;
int stableState = HIGH;
unsigned long lastChangeMs = 0;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) {
    lastChangeMs = millis();
    lastReading = reading;
  }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

void applyLevel() {
  int pwm = LEVELS[levelNo];
  analogWrite(LED_PIN, pwm);

  float duty = pwm * 100.0 / 255.0;
  float tonMs = PERIOD_MS * pwm / 255.0;

  Serial.printf("PWM=%d  Duty=%.1f%%  T=%.2fms  Ton=%.2fms\n",
                pwm, duty, PERIOD_MS, tonMs);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println("Ex6-1 PWM Duty (ESP32) - press button");
  applyLevel();
}

void loop() {
  if (buttonPressed()) {
    levelNo = (levelNo + 1) % LEVEL_COUNT;
    applyLevel();
  }
}
