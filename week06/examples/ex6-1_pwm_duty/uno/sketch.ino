/*
 * 예제 6-1  PWM 듀티비(Duty Cycle)와 LED 밝기  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : D9 → 330Ω → LED(+, 긴 다리) → LED(-, 짧은 다리) → GND
 *        D2 ↔ 푸시버튼 ↔ GND   (INPUT_PULLUP: 누르면 LOW)
 *
 * 동작 : 버튼을 누를 때마다 PWM 값이 0 → 32 → 64 → 128 → 192 → 255 로 바뀐다.
 *        시리얼 모니터에 PWM 값, 듀티비(%), HIGH 시간(Ton)을 출력한다.
 *
 * 핵심 : analogWrite(pin, value) 는 "아날로그 전압"이 아니라
 *        타이머가 만드는 사각파의 HIGH 비율(듀티비)을 바꾼다.
 *        UNO D9 는 Timer1 이 만드는 약 490 Hz PWM 이다 (주기 T ≈ 2.04 ms).
 */

const int LED_PIN = 9;          // Timer1 OC1A 핀 (PWM)
const int BTN_PIN = 2;          // 버튼 (다른 쪽은 GND)

const int LEVELS[] = {0, 32, 64, 128, 192, 255};
const int LEVEL_COUNT = sizeof(LEVELS) / sizeof(LEVELS[0]);

const float PWM_FREQ_HZ = 490.2;               // UNO D9 기본 PWM 주파수
const float PERIOD_MS   = 1000.0 / PWM_FREQ_HZ; // 주기 T (ms)

int levelNo = 0;

// --- 버튼 디바운스 (4주차 방식) ---
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
    return stableState == LOW;      // 눌리는 순간에만 true
  }
  return false;
}

void applyLevel() {
  int pwm = LEVELS[levelNo];
  analogWrite(LED_PIN, pwm);

  float duty = pwm * 100.0 / 255.0;            // 듀티비(%)
  float tonMs = PERIOD_MS * pwm / 255.0;       // HIGH 유지 시간

  Serial.print("PWM=");     Serial.print(pwm);
  Serial.print("  Duty=");  Serial.print(duty, 1);
  Serial.print("%  T=");    Serial.print(PERIOD_MS, 2);
  Serial.print("ms  Ton="); Serial.print(tonMs, 2);
  Serial.println("ms");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println("Ex6-1 PWM Duty (UNO) - press button");
  applyLevel();
}

void loop() {
  if (buttonPressed()) {
    levelNo = (levelNo + 1) % LEVEL_COUNT;
    applyLevel();
  }
}
