/*
 * 예제 6-3  타이머 CTC 모드로 소리 만들기 (tone() 직접 구현)  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : D9 → 수동부저(+) ,  수동부저(-) → GND           (Timer1 OC1A 출력)
 *        D2 ↔ 푸시버튼 ↔ GND                            (옥타브 전환)
 *
 * Timer1 설정 : CTC 모드(WGM12, 모드 4), 비교 일치 시 OC1A 토글(COM1A0)
 *   - 타이머 클록  f_tick = 16 MHz / 8 = 2 MHz   (1 tick = 0.5 µs)
 *   - 비교 일치마다 핀이 반전되므로 2번 일치해야 한 주기
 *   - 출력 주파수  f_out = 2 MHz / (2 × (OCR1A + 1)) = 1,000,000 / (OCR1A + 1)
 *   - 따라서 OCR1A = 1,000,000 / f - 1   (정수 → 약간의 오차 발생: 5주차 양자화와 같은 원리)
 *
 * 동작 : 도~높은 도를 400 ms 울림 + 100 ms 쉼으로 반복 (delay() 없이 millis 상태 머신)
 *        버튼을 누르면 한 옥타브 위(주파수 × 2)로 전환
 *
 * 참고 : Arduino 의 tone() 함수도 내부적으로 이와 같은 방식(UNO 는 Timer2)을 쓴다.
 *        수동부저는 "주파수가 있는 신호"를 넣어야 소리가 난다. 능동부저(내부 발진기)와 구분.
 */

const int BUZZER_PIN = 9;       // OC1A
const int BTN_PIN    = 2;

const unsigned int NOTES[] = {262, 294, 330, 349, 392, 440, 494, 523};
const char* NAMES[]        = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si", "Do'"};
const int NOTE_COUNT = sizeof(NOTES) / sizeof(NOTES[0]);

const unsigned long ON_MS  = 400;
const unsigned long OFF_MS = 100;

int  noteNo   = 0;
int  octave   = 1;              // 1 또는 2 (×2)
bool soundOn  = false;
unsigned long stateMs = 0;

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

void timerToneStart(unsigned int freqHz) {
  unsigned int ocr = (unsigned int)(1000000UL / freqHz - 1);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = ocr;
  TCCR1A = _BV(COM1A0);                 // 비교 일치 시 OC1A 토글
  TCCR1B = _BV(WGM12) | _BV(CS11);      // CTC(모드 4), 분주비 8 → 타이머 시작

  float realHz = 1000000.0 / (ocr + 1);
  Serial.print(NAMES[noteNo]);
  Serial.print("  target=");  Serial.print(freqHz);
  Serial.print("Hz  OCR1A="); Serial.print(ocr);
  Serial.print("  real=");    Serial.print(realHz, 2);
  Serial.println("Hz");
}

void timerToneStop() {
  TCCR1B = 0;                           // 클록 차단 → 타이머 정지
  TCCR1A = 0;                           // OC1A 연결 해제
  digitalWrite(BUZZER_PIN, LOW);        // 부저에 DC 가 걸리지 않게 LOW 고정
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println("Ex6-3 Timer1 CTC tone (UNO)");
  timerToneStart(NOTES[noteNo] * octave);
  soundOn = true;
  stateMs = millis();
}

void loop() {
  if (buttonPressed()) {
    octave = (octave == 1) ? 2 : 1;
    Serial.print("Octave x"); Serial.println(octave);
  }

  unsigned long now = millis();
  if (soundOn && now - stateMs >= ON_MS) {          // 울림 → 쉼
    timerToneStop();
    soundOn = false;
    stateMs = now;
  } else if (!soundOn && now - stateMs >= OFF_MS) { // 쉼 → 다음 음
    noteNo = (noteNo + 1) % NOTE_COUNT;
    timerToneStart(NOTES[noteNo] * octave);
    soundOn = true;
    stateMs = now;
  }
}
