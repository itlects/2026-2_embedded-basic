/*
 * 실습 5-1  ADC 기본값 읽기 & 샘플링 주기  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 5V / GND, 가운데(Wiper) → A0
 *        D2 ↔ 푸시버튼 ↔ GND   (INPUT_PULLUP: 누르면 LOW, 샘플링 주기 변경)
 *
 * 진행 : 1) 시작 시 analogRead() 1회에 걸리는 시간을 측정해 출력한다.
 *        2) 버튼으로 샘플링 주기 T = 10 / 100 / 500 / 1000 ms 를 바꾼다.
 *        3) 가변저항을 1초 안에 왕복시키며 Serial Plotter 로 그래프를 비교한다.
 *        4) 마지막에 Wiper 선을 뽑아 플로팅(떠 있는 입력) 잡음을 확인한다.
 *
 * 핵심 : 샘플링 주파수 fs = 1 / T.  T 가 길면 빠른 변화를 놓친다.
 *        delay() 대신 millis() 로 일정한 주기를 만든다 (4주차 논블로킹 구조).
 */

const int POT_PIN = A0;
const int BTN_PIN = 2;

const unsigned long PERIODS[] = {10, 100, 500, 1000};   // 샘플링 주기(ms)
const int PERIOD_COUNT = sizeof(PERIODS) / sizeof(PERIODS[0]);
int periodIdx = 1;                                      // 시작 100 ms
unsigned long lastSampleMs = 0;

// --- 버튼 디바운스 ---
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

// analogRead() 1회 시간 측정 (100회 평균)
void measureAdcTime() {
  const int N = 100;
  volatile int dummy = 0;
  unsigned long t0 = micros();
  for (int i = 0; i < N; i++) dummy = analogRead(POT_PIN);
  unsigned long dt = micros() - t0;
  (void)dummy;                                          // 최적화로 읽기가 사라지지 않게 volatile 사용
  float perRead = dt / (float)N;
  Serial.print(F("# analogRead 1회 = ")); Serial.print(perRead, 1);
  Serial.print(F(" us  → 최대 약 ")); Serial.print(1000000.0 / perRead, 0);
  Serial.println(F(" samples/s"));
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("# 실습 5-1 ADC 샘플링 (UNO) - 버튼으로 주기 변경"));
  measureAdcTime();
  Serial.print(F("# T = ")); Serial.print(PERIODS[periodIdx]);
  Serial.print(F(" ms, fs = ")); Serial.print(1000.0 / PERIODS[periodIdx], 1); Serial.println(F(" Hz"));
}

void loop() {
  if (buttonPressed()) {                                 // 주기 변경
    periodIdx = (periodIdx + 1) % PERIOD_COUNT;
    Serial.print(F("# T = ")); Serial.print(PERIODS[periodIdx]);
    Serial.print(F(" ms, fs = ")); Serial.print(1000.0 / PERIODS[periodIdx], 1); Serial.println(F(" Hz"));
  }

  if (millis() - lastSampleMs >= PERIODS[periodIdx]) {   // 일정 주기 샘플링
    lastSampleMs = millis();
    int adc = analogRead(POT_PIN);                       // 0 ~ 1023
    Serial.print(F("Min:0,Max:1023,ADC:"));              // Plotter 축 고정
    Serial.println(adc);
  }
}
