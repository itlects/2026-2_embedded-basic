// Week 07 - 예제 2B: 타이머 인터럽트 시계 (ESP32 DevKit)
// 외부 부품 없음. USB 연결 후 시리얼 모니터 115200bps.
//
// ESP32 하드웨어 타이머를 1 MHz(1틱 = 1 us)로 동작시키고
// 1,000,000틱(= 1초)마다 알람 인터럽트를 발생시킨다(자동 재장전).
// Arduino-ESP32 코어 3.x API(timerBegin(freq), timerAlarm)를 기본으로 사용하며,
// 코어 2.x를 설치한 경우 아래 #else 부분이 컴파일된다.

hw_timer_t *timer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrSeconds = 0;   // ISR이 증가시키는 초 카운터
volatile bool tickFlag = false;     // "1초가 지났다" 이벤트 플래그

// 타이머 알람 ISR: IRAM에 배치(ARDUINO_ISR_ATTR), 짧게 카운트와 플래그만 갱신
void ARDUINO_ISR_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  isrSeconds++;
  tickFlag = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setupTimer1Hz() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  timer = timerBegin(1000000);                 // 타이머 주파수 1 MHz
  timerAttachInterrupt(timer, &onTimer);       // ISR 연결
  timerAlarm(timer, 1000000, true, 0);         // 1,000,000틱마다, 자동 재장전, 무한 반복
#else
  timer = timerBegin(0, 80, true);             // 타이머0, 80MHz/80 = 1 MHz, 상향 카운트
  timerAttachInterrupt(timer, &onTimer, true); // 에지 방식
  timerAlarmWrite(timer, 1000000, true);       // 1초, 자동 재장전
  timerAlarmEnable(timer);
#endif
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 hardware timer 1Hz interrupt clock");
  setupTimer1Hz();
}

void loop() {
  bool tick;
  uint32_t seconds;

  // ESP32는 듀얼코어이므로 noInterrupts() 대신 스핀락 임계구역으로 공유 변수를 복사한다.
  portENTER_CRITICAL(&timerMux);
  tick = tickFlag;
  tickFlag = false;
  seconds = isrSeconds;
  portEXIT_CRITICAL(&timerMux);

  if (tick) {
    uint32_t hh = (seconds / 3600UL) % 24UL;
    uint32_t mm = (seconds / 60UL) % 60UL;
    uint32_t ss = seconds % 60UL;
    Serial.printf("TIME %02lu:%02lu:%02lu\n",
                  (unsigned long)hh, (unsigned long)mm, (unsigned long)ss);
  }
}
