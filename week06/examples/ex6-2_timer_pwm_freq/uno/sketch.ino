/*
 * 예제 6-2  타이머 레지스터로 PWM 주파수·듀티 직접 설정  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : D9  → 330Ω → LED(+) → LED(-) → GND        (Timer1 OC1A 출력)
 *        가변저항: 양 끝 5V / GND, 가운데(Wiper) → A0   (듀티 조절)
 *        D2 ↔ 푸시버튼 ↔ GND                         (주파수 변경)
 *
 * Timer1 설정 : Fast PWM, TOP = ICR1  (WGM13:10 = 1110, 모드 14)
 *   - 타이머 클록  f_tick = 16 MHz / 분주비(256) = 62,500 Hz  (1 tick = 16 µs)
 *   - PWM 주파수   f_pwm  = f_tick / (ICR1 + 1)
 *   - 듀티비       Duty   = (OCR1A + 1) / (ICR1 + 1)
 *
 * 관찰 : 1 Hz, 4 Hz 에서는 LED가 "깜빡임"으로 보이고,
 *        100 Hz 이상에서는 같은 듀티라도 "밝기"로 보인다 (잔상 효과).
 *
 * 주의 : 이 예제는 Timer1 을 직접 설정하므로 D9·D10 의 analogWrite(),
 *        Servo 라이브러리와 함께 쓸 수 없다.
 */

const int LED_PIN = 9;          // OC1A
const int POT_PIN = A0;
const int BTN_PIN = 2;

const unsigned long TICK_HZ = 62500UL;          // 16 MHz / 256
const unsigned int  FREQS[] = {1, 4, 20, 100, 1000};
const int FREQ_COUNT = sizeof(FREQS) / sizeof(FREQS[0]);
int freqNo = 0;

unsigned long lastUpdateMs = 0;
unsigned long lastPrintMs  = 0;

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

// Timer1 을 Fast PWM(모드 14)으로 초기화
void timer1Init() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1A = _BV(COM1A1) | _BV(WGM11);            // OC1A 비반전 출력, WGM11
  TCCR1B = _BV(WGM13) | _BV(WGM12);             // WGM13, WGM12 → 모드 14
  setPwmFrequency(FREQS[freqNo]);
}

// 주파수 변경: ICR1(TOP) 값을 다시 계산한다
void setPwmFrequency(unsigned int freqHz) {
  unsigned int top = (unsigned int)(TICK_HZ / freqHz - 1);
  TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));  // 타이머 정지
  TCNT1 = 0;                                        // 카운터 초기화
  ICR1  = top;                                      // 새 TOP
  OCR1A = top / 2;                                  // 일단 50%
  TCCR1B |= _BV(CS12);                              // 분주비 256 으로 재시작
}

// 듀티 설정: Fast PWM 은 OCR1A = 0 이어도 1 tick 동안 HIGH 가 나온다.
// 완전히 끄려면 OC1A 출력을 잠시 끊고 LOW 로 고정한다.
void setDuty(unsigned int ocr) {
  if (ocr == 0) {
    TCCR1A &= ~_BV(COM1A1);                         // OC1A 분리 → 일반 GPIO
    digitalWrite(LED_PIN, LOW);
  } else {
    OCR1A = ocr;
    TCCR1A |= _BV(COM1A1);                          // OC1A 다시 연결
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);                         // OC1A 출력 허용
  pinMode(BTN_PIN, INPUT_PULLUP);
  timer1Init();
  Serial.println("Ex6-2 Timer1 register PWM (UNO)");
}

void loop() {
  if (buttonPressed()) {
    freqNo = (freqNo + 1) % FREQ_COUNT;
    setPwmFrequency(FREQS[freqNo]);
  }

  // 20 ms 마다 가변저항 값으로 듀티(OCR1A) 갱신
  if (millis() - lastUpdateMs >= 20) {
    lastUpdateMs = millis();
    int adc = analogRead(POT_PIN);                  // 0 ~ 1023
    unsigned long top = ICR1;
    setDuty((unsigned int)((top * adc) / 1023UL));  // 0 ~ TOP
  }

  // 500 ms 마다 레지스터 값 출력
  if (millis() - lastPrintMs >= 500) {
    lastPrintMs = millis();
    unsigned int top = ICR1, cmp = OCR1A;
    bool on = TCCR1A & _BV(COM1A1);
    float duty = on ? (cmp + 1) * 100.0 / (top + 1) : 0.0;
    if (!on) cmp = 0;
    Serial.print("f=");      Serial.print(FREQS[freqNo]);
    Serial.print("Hz  ICR1="); Serial.print(top);
    Serial.print("  OCR1A=");  Serial.print(cmp);
    Serial.print("  Duty=");   Serial.print(duty, 1);
    Serial.println("%");
  }
}
