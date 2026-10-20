// Week 07 - 실습 3: 타이머 인터럽트 시계 + 3버튼 인터럽트 카운터 (Arduino UNO)
// 버튼: D2 = 증가(A), D3 = 초기화(B), D4 = 일시정지/재개(C)
// 각 버튼: 입력핀 <-> 스위치 <-> GND (내부 풀업 사용). 시리얼 115200bps.
//
// 사용하는 인터럽트 벡터 2개
//   PCINT2_vect       : 포트 D 핀 변경 인터럽트(D2·D3·D4) -> 버튼 이벤트
//   TIMER1_COMPA_vect : Timer1 비교일치(1초)              -> 시계
#include <avr/interrupt.h>

const uint8_t BTN[3] = {2, 3, 4};
const uint32_t DEBOUNCE_MS = 30;

// ---- ISR과 공유하는 변수(volatile) ----
volatile uint8_t lastPortD = 0;     // 직전 PIND 값
volatile uint8_t edgeBits = 0;      // bit0=A, bit1=B, bit2=C 하강 에지 발생
volatile uint32_t isrSeconds = 0;   // 타이머 ISR이 증가시키는 초
volatile bool tickFlag = false;     // 1초 경과 이벤트

// 핀 변경 인터럽트: D2~D4 중 어느 핀이 HIGH->LOW가 되었는지 비트로 기록
ISR(PCINT2_vect) {
  uint8_t current = PIND;
  uint8_t falling = (uint8_t)(lastPortD & (uint8_t)~current);  // 1 -> 0 으로 바뀐 비트
  edgeBits |= (falling >> 2) & 0x07;                            // PD2..PD4 -> bit0..bit2
  lastPortD = current;
}

// 타이머1 비교일치 인터럽트: 1초마다 실행
ISR(TIMER1_COMPA_vect) {
  isrSeconds++;
  tickFlag = true;
}

// ---- loop()에서만 사용하는 변수 ----
bool awaiting[3] = {false, false, false};  // 에지 후 안정화 대기 중인가
uint32_t lowSince[3] = {0, 0, 0};          // LOW가 시작된 시각(ms), 0 = 미측정
uint32_t count = 0;
bool paused = false;

void setupButtonsPCINT() {
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(BTN[i], INPUT_PULLUP);
  }
  noInterrupts();
  lastPortD = PIND;
  PCMSK2 |= _BV(PCINT18) | _BV(PCINT19) | _BV(PCINT20);  // D2, D3, D4 변화 감시
  PCIFR  |= _BV(PCIF2);                                   // 남아 있는 플래그 제거
  PCICR  |= _BV(PCIE2);                                   // PCINT2 그룹 허용
  interrupts();
}

void setupTimer1_1Hz() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = 62499;                     // 16MHz/256 = 62.5kHz -> 62,500카운트 = 1초
  TCCR1B = _BV(WGM12) | _BV(CS12);    // CTC 모드, 분주 256
  TIFR1  = _BV(OCF1A);
  TIMSK1 = _BV(OCIE1A);               // 비교일치 A 인터럽트 허용
  interrupts();
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

void printCounter() {
  Serial.print("COUNT ");
  Serial.print(count);
  Serial.print(" PAUSE ");
  Serial.println(paused ? "ON" : "OFF");
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
  setupButtonsPCINT();
  setupTimer1_1Hz();
  Serial.println("UNO: D2 increment, D3 reset, D4 pause/resume (Timer1 clock)");
  printCounter();
}

void loop() {
  uint32_t now = millis();

  // 1) ISR이 남긴 이벤트를 한 번에 복사하고 비운다(원자적 복사)
  uint8_t edges;
  bool tick;
  uint32_t seconds;
  noInterrupts();
  edges = edgeBits;
  edgeBits = 0;
  tick = tickFlag;
  tickFlag = false;
  seconds = isrSeconds;
  interrupts();

  // 2) 시계 출력
  if (tick) printClock(seconds);

  // 3) 버튼 디바운스: 에지 후 30ms 연속 LOW일 때 1회 인정
  for (uint8_t i = 0; i < 3; i++) {
    if (edges & (1U << i)) {
      awaiting[i] = true;
      lowSince[i] = 0;
    }
    if (!awaiting[i]) continue;

    if (digitalRead(BTN[i]) == HIGH) {   // 바운스로 다시 HIGH -> 측정 재시작
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
