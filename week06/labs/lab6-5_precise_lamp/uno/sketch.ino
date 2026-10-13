/*
 * 실습 6-5  타이머 기반 정밀 가변램프  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : D9  → 330Ω → LED1(+) → GND          램프 LED  (Timer1 OC1A, 레지스터 PWM)
 *        D6  → 330Ω → LED2(+) → GND          심장박동 LED (Timer2 틱으로 0.5 s 토글)
 *        가변저항: 양 끝 5V / GND, Wiper → A0  밝기 조절
 *        D2  ↔ 푸시버튼 ↔ GND                PWM 주파수 변경 (2 → 10 → 50 → 500 Hz)
 *
 * 타이머 자원 배분 (UNO 는 타이머가 3개뿐이다)
 *   Timer0 : millis()/micros()  — 건드리지 않음
 *   Timer1 : 램프 LED PWM (Fast PWM 모드 14, TOP = ICR1, 분주비 256)
 *   Timer2 : 1 ms 시간 기준 인터럽트 (CTC, 분주비 128, OCR2A = 124)
 *            → 10 ms 마다 ADC 샘플링(100 Hz), 500 ms 마다 LED2 토글
 *   ⇒ tone() 은 Timer2 를 쓰므로 이 실습에서는 사용할 수 없다.
 *
 * 요구사항
 *   1) 가변저항을 정확히 100 Hz 로 샘플링하고 최근 8개 평균으로 잡음을 줄인다.
 *   2) 평균값으로 LED1 듀티(OCR1A)를 바꾼다.
 *   3) 버튼으로 PWM 주파수를 바꿔 2 Hz 에서는 깜빡임, 500 Hz 에서는 밝기로 보이는지 확인한다.
 *   4) LED2 는 loop 가 아무리 바빠도 정확히 0.5 s 마다 토글된다.
 *   5) loop() 에서 delay() 사용 금지. 500 ms 마다 상태를 시리얼로 출력한다.
 */

const int LAMP_PIN  = 9;        // OC1A
const int BEAT_PIN  = 6;
const int POT_PIN   = A0;
const int BTN_PIN   = 2;

const unsigned long TICK1_HZ = 62500UL;          // Timer1: 16 MHz / 256
const unsigned int  FREQS[]  = {2, 10, 50, 500};
const int FREQ_COUNT = sizeof(FREQS) / sizeof(FREQS[0]);
int freqNo = 3;                                  // 500 Hz 에서 시작

// ---------------- Timer2 ISR (1 ms) ----------------
const int AVG_N = 8;
volatile int  sampleBuf[AVG_N];
volatile int  bufIdx = 0;
volatile long bufSum = 0;
volatile bool newSample = false;
volatile unsigned int sampleCount = 0;
volatile unsigned int tick10 = 0, tick500 = 0;
volatile bool beatState = false;

ISR(TIMER2_COMPA_vect) {
  if (++tick10 >= 10) {                          // 10 ms → 100 Hz 샘플링
    tick10 = 0;
    int adc = analogRead(POT_PIN);
    bufSum += adc - sampleBuf[bufIdx];           // 이동평균 합 갱신
    sampleBuf[bufIdx] = adc;
    bufIdx = (bufIdx + 1) % AVG_N;
    newSample = true;
    sampleCount++;
  }
  if (++tick500 >= 500) {                        // 500 ms → 심장박동
    tick500 = 0;
    beatState = !beatState;
    digitalWrite(BEAT_PIN, beatState);
  }
}

void timer2Init() {
  cli();
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS22) | _BV(CS20);
  OCR2A  = 124;
  TCNT2  = 0;
  TIMSK2 = _BV(OCIE2A);
  sei();
}

// ---------------- Timer1 PWM ----------------
void setLampFrequency(unsigned int freqHz) {
  unsigned int top = (unsigned int)(TICK1_HZ / freqHz - 1);
  TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
  TCNT1 = 0;
  ICR1  = top;
  OCR1A = 0;
  TCCR1B |= _BV(CS12);
}

void timer1Init() {
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12);
  setLampFrequency(FREQS[freqNo]);
}

// 듀티 설정: Fast PWM 은 OCR1A = 0 이어도 1 tick 동안 HIGH 가 나온다.
// 완전히 끄려면 OC1A 출력을 잠시 끊고 LOW 로 고정한다.
void setDuty(unsigned int ocr) {
  if (ocr == 0) {
    TCCR1A &= ~_BV(COM1A1);                         // OC1A 분리 → 일반 GPIO
    digitalWrite(LAMP_PIN, LOW);
  } else {
    OCR1A = ocr;
    TCCR1A |= _BV(COM1A1);                          // OC1A 다시 연결
  }
}

// ---------------- 버튼 ----------------
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

int avgAdc = 0;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(BEAT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  timer1Init();
  timer2Init();
  Serial.println("Lab6-5 Precise Lamp (UNO)");
}

void loop() {
  if (buttonPressed()) {
    freqNo = (freqNo + 1) % FREQ_COUNT;
    setLampFrequency(FREQS[freqNo]);
  }

  if (newSample) {                               // 새 샘플이 들어왔을 때만 갱신
    noInterrupts();
    long sum = bufSum;
    newSample = false;
    interrupts();

    avgAdc = sum / AVG_N;
    unsigned long top = ICR1;
    setDuty((unsigned int)((top * avgAdc) / 1023UL));
  }

  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    noInterrupts();
    unsigned int cnt = sampleCount;
    sampleCount = 0;
    interrupts();

    unsigned int top = ICR1, cmp = OCR1A;
    bool on = TCCR1A & _BV(COM1A1);
    float duty = on ? (cmp + 1) * 100.0 / (top + 1) : 0.0;
    if (!on) cmp = 0;
    Serial.print("f=");       Serial.print(FREQS[freqNo]);
    Serial.print("Hz ADC=");  Serial.print(avgAdc);
    Serial.print(" ICR1=");   Serial.print(top);
    Serial.print(" OCR1A=");  Serial.print(cmp);
    Serial.print(" Duty=");   Serial.print(duty, 1);
    Serial.print("% Samples/0.5s="); Serial.println(cnt);   // 50 이 정상
  }
}
