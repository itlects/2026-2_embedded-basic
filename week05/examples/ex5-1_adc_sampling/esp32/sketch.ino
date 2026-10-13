/*
 * 실습 5-1  ADC 기본값 읽기 & 샘플링 주기  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 3.3V / GND, 가운데(Wiper) → GPIO34   (5V 연결 금지)
 *        GPIO13 ↔ 푸시버튼 ↔ GND   (INPUT_PULLUP, 샘플링 주기 변경)
 *
 * 진행 : 1) 시작 시 analogRead() 1회에 걸리는 시간을 측정해 UNO 와 비교한다.
 *        2) 버튼으로 샘플링 주기 T = 10 / 100 / 500 / 1000 ms 를 바꾼다.
 *        3) 가변저항을 1초 안에 왕복시키며 Serial Plotter 로 그래프를 비교한다.
 *        4) 마지막에 Wiper 선을 뽑아 플로팅 잡음을 확인한다.
 *
 * ESP32 : 12-bit ADC (0 ~ 4095). GPIO34 는 ADC1, 입력 전용 핀이다.
 */

const int POT_PIN = 34;
const int BTN_PIN = 13;                                 // GPIO34~39 는 풀업이 없어 버튼 금지

const unsigned long PERIODS[] = {10, 100, 500, 1000};
const int PERIOD_COUNT = sizeof(PERIODS) / sizeof(PERIODS[0]);
int periodIdx = 1;
unsigned long lastSampleMs = 0;

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

void measureAdcTime() {
  const int N = 100;
  volatile int dummy = 0;
  unsigned long t0 = micros();
  for (int i = 0; i < N; i++) dummy = analogRead(POT_PIN);
  unsigned long dt = micros() - t0;
  (void)dummy;                                          // 최적화로 읽기가 사라지지 않게 volatile 사용
  float perRead = dt / (float)N;
  Serial.printf("# analogRead 1회 = %.1f us  → 최대 약 %.0f samples/s\n",
                perRead, 1000000.0 / perRead);
}

void printPeriod() {
  Serial.printf("# T = %lu ms, fs = %.1f Hz\n", PERIODS[periodIdx], 1000.0 / PERIODS[periodIdx]);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);                             // 0 ~ 4095
  Serial.println("# 실습 5-1 ADC 샘플링 (ESP32) - 버튼으로 주기 변경");
  measureAdcTime();
  printPeriod();
}

void loop() {
  if (buttonPressed()) {
    periodIdx = (periodIdx + 1) % PERIOD_COUNT;
    printPeriod();
  }

  if (millis() - lastSampleMs >= PERIODS[periodIdx]) {
    lastSampleMs = millis();
    int adc = analogRead(POT_PIN);                       // 0 ~ 4095
    Serial.printf("Min:0,Max:4095,ADC:%d\n", adc);       // Plotter 축 고정
  }
}
