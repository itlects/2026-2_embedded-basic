// Week 07 - 예제 2B: 타이머 인터럽트 시계 (Arduino UNO / ATmega328P)
// 외부 부품 없음. USB 연결 후 시리얼 모니터 115200bps.
//
// Timer1을 CTC(Clear Timer on Compare match) 모드로 설정한다.
//   16 MHz / 256(분주) = 62,500 Hz  -> 1카운트 = 16 us
//   OCR1A = 62,500 - 1 = 62,499     -> 62,500카운트마다 = 정확히 1초
// TCNT1 == OCR1A 가 되는 순간 OCF1A 플래그가 세트되고,
// 하드웨어가 벡터 테이블의 TIMER1_COMPA_vect(벡터 번호 11) 위치로 점프하여 ISR을 실행한다.
//
// 주의: UNO의 16MHz 클럭은 세라믹 공진자라 수백 ppm 오차가 있을 수 있다.
//       장시간 정확한 시각이 필요하면 RTC 모듈(DS3231 등)을 사용한다.

volatile uint32_t isrSeconds = 0;   // ISR이 증가시키는 초 카운터 (4바이트)
volatile bool tickFlag = false;     // "1초가 지났다" 이벤트 플래그

// 타이머1 비교일치 A 인터럽트 서비스 루틴: 짧게 카운트와 플래그만 갱신
ISR(TIMER1_COMPA_vect) {
  isrSeconds++;
  tickFlag = true;
}

void setupTimer1_1Hz() {
  noInterrupts();               // 설정 중 인터럽트 차단
  TCCR1A = 0;                   // Arduino 코어가 PWM용으로 설정해 둔 값을 초기화
  TCCR1B = 0;
  TCNT1  = 0;                   // 카운터 0부터 시작
  OCR1A  = 62499;               // 비교값: 62,500카운트 = 1초
  TCCR1B = _BV(WGM12)           // CTC 모드(Mode 4, TOP = OCR1A)
         | _BV(CS12);           // 분주비 256 -> 타이머 시작
  TIFR1  = _BV(OCF1A);          // 이전에 남은 비교일치 플래그 제거(1을 써서 클리어)
  TIMSK1 = _BV(OCIE1A);         // 비교일치 A 인터럽트 허용
  interrupts();                 // 전역 인터럽트 허용(SREG의 I비트 = 1)
}

void printClock(uint32_t totalSeconds) {
  uint32_t hh = (totalSeconds / 3600UL) % 24UL;
  uint32_t mm = (totalSeconds / 60UL) % 60UL;
  uint32_t ss = totalSeconds % 60UL;
  char line[24];
  snprintf(line, sizeof(line), "TIME %02lu:%02lu:%02lu",
           (unsigned long)hh, (unsigned long)mm, (unsigned long)ss);
  Serial.println(line);
}

void setup() {
  Serial.begin(115200);
  Serial.println("UNO Timer1 CTC 1Hz interrupt clock");
  setupTimer1_1Hz();
}

void loop() {
  bool tick;
  uint32_t seconds;

  // 4바이트 변수는 8비트 AVR에서 한 번에 읽히지 않으므로
  // 인터럽트를 잠시 막고 복사한다(원자적 복사).
  noInterrupts();
  tick = tickFlag;
  tickFlag = false;
  seconds = isrSeconds;
  interrupts();

  if (tick) {
    printClock(seconds);        // Serial 출력은 ISR이 아니라 loop()에서 수행
  }

  // 여기에 다른 작업을 넣어도 시계는 타이머 하드웨어가 유지한다.
}
