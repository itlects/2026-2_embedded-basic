// Week 07 - 실습 3: 타이머 인터럽트 시계 + 3버튼 인터럽트 카운터 (ESP32 DevKit)
// 버튼: GPIO18 = 증가(A), GPIO19 = 초기화(B), GPIO21 = 일시정지/재개(C)
// 각 버튼: GPIO <-> 스위치 <-> GND (내부 풀업 사용). 시리얼 115200bps.
// Arduino-ESP32 코어 3.x 기준(2.x도 #else 부분으로 컴파일됨). 외부 라이브러리 없음.

const uint8_t BTN[3] = {18, 19, 21};
const uint32_t DEBOUNCE_MS = 30;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;   // ISR과 loop() 공유 데이터 보호
hw_timer_t *timer = nullptr;

// ---- ISR과 공유하는 변수(volatile) ----
volatile uint32_t edgeBits = 0;     // bit0=A, bit1=B, bit2=C
volatile uint32_t isrSeconds = 0;
volatile bool tickFlag = false;

// 버튼별 GPIO 인터럽트 ISR: 비트 플래그만 기록
void ARDUINO_ISR_ATTR isrInc()   { portENTER_CRITICAL_ISR(&mux); edgeBits |= 1U; portEXIT_CRITICAL_ISR(&mux); }
void ARDUINO_ISR_ATTR isrReset() { portENTER_CRITICAL_ISR(&mux); edgeBits |= 2U; portEXIT_CRITICAL_ISR(&mux); }
void ARDUINO_ISR_ATTR isrPause() { portENTER_CRITICAL_ISR(&mux); edgeBits |= 4U; portEXIT_CRITICAL_ISR(&mux); }

// 하드웨어 타이머 ISR: 1초마다 실행
void ARDUINO_ISR_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&mux);
  isrSeconds++;
  tickFlag = true;
  portEXIT_CRITICAL_ISR(&mux);
}

// ---- loop()에서만 사용하는 변수 ----
bool awaiting[3] = {false, false, false};
uint32_t lowSince[3] = {0, 0, 0};
uint32_t count = 0;
bool paused = false;

void setupTimer1Hz() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  timer = timerBegin(1000000);                 // 1 MHz
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);         // 1초, 자동 재장전
#else
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
#endif
}

void printCounter() {
  Serial.printf("COUNT %lu PAUSE %s\n", (unsigned long)count, paused ? "ON" : "OFF");
}

void handleButton(uint8_t i) {
  if (i == 0) {                 // A: 증가 (일시정지 중에는 무시)
    if (!paused) count++;
  } else if (i == 1) {          // B: 초기화
    count = 0;
  } else {                      // C: 일시정지/재개 토글
    paused = !paused;
  }
  printCounter();
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 3; i++) pinMode(BTN[i], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN[0]), isrInc, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN[1]), isrReset, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN[2]), isrPause, FALLING);
  setupTimer1Hz();
  Serial.println("ESP32: GPIO18 increment, GPIO19 reset, GPIO21 pause/resume (HW timer clock)");
  printCounter();
}

void loop() {
  uint32_t now = millis();

  // 1) ISR이 남긴 이벤트를 임계구역에서 복사하고 비운다
  uint32_t edges, seconds;
  bool tick;
  portENTER_CRITICAL(&mux);
  edges = edgeBits;
  edgeBits = 0;
  tick = tickFlag;
  tickFlag = false;
  seconds = isrSeconds;
  portEXIT_CRITICAL(&mux);

  // 2) 시계 출력
  if (tick) {
    Serial.printf("TIME %02lu:%02lu:%02lu\n",
                  (unsigned long)((seconds / 3600UL) % 24UL),
                  (unsigned long)((seconds / 60UL) % 60UL),
                  (unsigned long)(seconds % 60UL));
  }

  // 3) 버튼 디바운스: 에지 후 30ms 연속 LOW일 때 1회 인정
  for (uint8_t i = 0; i < 3; i++) {
    if (edges & (1U << i)) {
      awaiting[i] = true;
      lowSince[i] = 0;
    }
    if (!awaiting[i]) continue;

    if (digitalRead(BTN[i]) == HIGH) {
      lowSince[i] = 0;
      continue;
    }
    if (lowSince[i] == 0) lowSince[i] = now;
    if ((uint32_t)(now - lowSince[i]) < DEBOUNCE_MS) continue;

    awaiting[i] = false;
    lowSince[i] = 0;
    handleButton(i);
  }
}
