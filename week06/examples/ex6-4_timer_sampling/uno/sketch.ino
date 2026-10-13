/*
 * 예제 6-4  하드웨어 타이머 인터럽트로 "정확한 샘플링 주기" 만들기  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 가변저항  양 끝 5V / GND, 가운데(Wiper) → A0
 *        D2 ↔ 푸시버튼 ↔ GND         (loop 부하 ON/OFF)
 *        내장 LED(D13)                (부하 ON 표시, 배선 불필요)
 *
 * 비교하는 두 가지 샘플링 방식 (목표 주기 10 ms = 100 Hz)
 *   (A) Timer2 인터럽트 : 1 ms 마다 하드웨어가 ISR 을 호출 → 10번째마다 ADC 읽기
 *   (B) loop + millis() : 5주차 방식. loop 가 바쁘면 검사가 늦어진다
 *
 * Timer2 설정 : CTC 모드(WGM21), 분주비 128, OCR2A = 124
 *   f_int = 16 MHz / 128 / (124 + 1) = 1,000 Hz  → 1 ms 마다 TIMER2_COMPA_vect
 *
 * 실험 : 버튼으로 loop 에 7 ms 짜리 "무거운 작업"을 넣었다 뺐다 하면서
 *        Serial Plotter 에서 TimerMax / LoopMax (µs) 를 비교한다.
 *
 * 주의 : Timer2 를 사용하므로 이 예제에서는 tone(), D3·D11 의 analogWrite() 를 쓸 수 없다.
 *        ISR 안에서는 Serial 출력·delay() 를 하지 않는다 (짧게!).
 */

const int POT_PIN = A0;
const int BTN_PIN = 2;
const int LOAD_LED = LED_BUILTIN;      // D13

const unsigned int SAMPLE_MS = 10;     // 목표 샘플링 주기

// ---------- ISR 과 loop 가 함께 쓰는 변수는 volatile ----------
volatile unsigned int  isrTick   = 0;
volatile int           isrAdc    = 0;
volatile unsigned long isrLastUs = 0;
volatile unsigned long isrMinUs  = 0xFFFFFFFF, isrMaxUs = 0;
volatile unsigned int  isrCount  = 0;

ISR(TIMER2_COMPA_vect) {               // 1 ms 마다 하드웨어가 호출
  if (++isrTick < SAMPLE_MS) return;
  isrTick = 0;

  unsigned long now = micros();
  isrAdc = analogRead(POT_PIN);        // UNO 는 약 112 µs (ISR 안에서 허용 가능한 길이)

  if (isrLastUs != 0) {
    unsigned long dt = now - isrLastUs;
    if (dt < isrMinUs) isrMinUs = dt;
    if (dt > isrMaxUs) isrMaxUs = dt;
  }
  isrLastUs = now;
  isrCount++;
}

void timer2Init() {
  cli();                               // 설정 중 인터럽트 금지
  TCCR2A = _BV(WGM21);                 // CTC 모드
  TCCR2B = _BV(CS22) | _BV(CS20);      // 분주비 128
  OCR2A  = 124;                        // 125 카운트 = 1 ms
  TCNT2  = 0;
  TIMSK2 = _BV(OCIE2A);                // 비교 일치 A 인터럽트 허용
  sei();
}

// ---------- (B) loop + millis 방식 : 타이밍만 측정 ----------
// (ADC 는 ISR 에서 읽고 있으므로 여기서 analogRead 를 동시에 호출하지 않는다)
unsigned long loopLastMs = 0, loopLastUs = 0;
unsigned long loopMinUs = 0xFFFFFFFF, loopMaxUs = 0;

// ---------- 버튼 ----------
const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH, stableState = HIGH;
unsigned long lastChangeMs = 0;
bool heavyLoad = false;

bool buttonPressed() {
  int reading = digitalRead(BTN_PIN);
  if (reading != lastReading) { lastChangeMs = millis(); lastReading = reading; }
  if (millis() - lastChangeMs >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

unsigned long lastReportMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LOAD_LED, OUTPUT);
  timer2Init();
}

void loop() {
  if (buttonPressed()) {
    heavyLoad = !heavyLoad;
    digitalWrite(LOAD_LED, heavyLoad);
  }

  // (B) millis 기반 10 ms 주기 검사
  if (millis() - loopLastMs >= SAMPLE_MS) {
    loopLastMs = millis();
    unsigned long now = micros();
    if (loopLastUs != 0) {
      unsigned long dt = now - loopLastUs;
      if (dt < loopMinUs) loopMinUs = dt;
      if (dt > loopMaxUs) loopMaxUs = dt;
    }
    loopLastUs = now;
  }

  // 무거운 작업 흉내 (예: 긴 계산, 많은 Serial 출력, LCD 갱신)
  if (heavyLoad) delayMicroseconds(7000);

  // 1초마다 결과 출력 (Serial Plotter 용 "이름:값" 형식)
  if (millis() - lastReportMs >= 1000) {
    lastReportMs = millis();

    noInterrupts();                    // ISR 변수를 일관되게 복사
    unsigned long tMin = isrMinUs, tMax = isrMaxUs;
    unsigned int  cnt  = isrCount;
    int adc = isrAdc;
    isrMinUs = 0xFFFFFFFF; isrMaxUs = 0; isrCount = 0;
    interrupts();

    Serial.print("Target:");   Serial.print(SAMPLE_MS * 1000UL);
    Serial.print(",TimerMin:"); Serial.print(tMin);
    Serial.print(",TimerMax:"); Serial.print(tMax);
    Serial.print(",LoopMin:");  Serial.print(loopMinUs);
    Serial.print(",LoopMax:");  Serial.print(loopMaxUs);
    Serial.print(",Samples:");  Serial.print(cnt);
    Serial.print(",ADC:");      Serial.print(adc);
    Serial.print(",Load:");     Serial.println(heavyLoad ? 1 : 0);

    loopMinUs = 0xFFFFFFFF; loopMaxUs = 0;
  }
}
