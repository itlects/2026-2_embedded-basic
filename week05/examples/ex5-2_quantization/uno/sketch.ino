/*
 * 실습 5-2  전압 환산 & 양자화 실험  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 5-1 과 동일 (가변저항 Wiper → A0, 버튼 D2 ↔ GND)
 *
 * 동작 : 10-bit 로 읽은 값을 오른쪽 시프트해서 8 / 6 / 4 / 2-bit ADC 를 흉내 낸다.
 *        버튼으로 비트 수를 바꾸며 Serial Plotter 에서 비교한다.
 *          V      : 10-bit 로 추정한 전압 (매끈한 선)
 *          Vq     : b-bit 코드로 복원한 전압 (계단)
 *          Err_mV : 양자화 오차 V - Vq (mV, 항상 ±½ LSB 이내)
 *
 * 공식 : 1 LSB = Vref / 2^b,  복원 전압 = (code + 0.5) × LSB
 */

const int POT_PIN = A0;
const int BTN_PIN = 2;
const float VREF = 5.0;
const int ADC_BITS = 10;

const int BITS[] = {10, 8, 6, 4, 2};                    // 흉내 낼 비트 수
const int BITS_COUNT = sizeof(BITS) / sizeof(BITS[0]);
int bitsIdx = 3;                                        // 4-bit 에서 시작

unsigned long lastSampleMs = 0;
float maxErrMv = 0;                                     // 현재 비트 수에서 관측한 최대 |오차|

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

void printInfo() {
  int b = BITS[bitsIdx];
  float lsbMv = VREF * 1000.0 / (1L << b);
  Serial.print(F("# ")); Serial.print(b); Serial.print(F("-bit : 단계 수 "));
  Serial.print(1L << b); Serial.print(F(", 1 LSB = ")); Serial.print(lsbMv, 2);
  Serial.print(F(" mV, 최대 오차 ±")); Serial.print(lsbMv / 2, 2); Serial.println(F(" mV"));
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("# 실습 5-2 양자화 (UNO) - 버튼으로 비트 수 변경"));
  printInfo();
}

void loop() {
  if (buttonPressed()) {
    bitsIdx = (bitsIdx + 1) % BITS_COUNT;
    maxErrMv = 0;
    printInfo();
  }

  if (millis() - lastSampleMs >= 50) {                  // 20 Hz 샘플링
    lastSampleMs = millis();
    int raw  = analogRead(POT_PIN);                     // 0 ~ 1023
    float v  = (raw + 0.5) * VREF / 1024.0;             // 전압 추정
    int b    = BITS[bitsIdx];
    int code = raw >> (ADC_BITS - b);                   // b-bit 코드
    float lsb = VREF / (1L << b);                       // 1 LSB (V)
    float vq = (code + 0.5) * lsb;                      // 복원 전압
    float errMv = (v - vq) * 1000.0;                    // 양자화 오차
    if (fabs(errMv) > maxErrMv) maxErrMv = fabs(errMv);

    Serial.print(F("V:"));       Serial.print(v, 3);
    Serial.print(F(",Vq:"));     Serial.print(vq, 3);
    Serial.print(F(",Err_mV:")); Serial.print(errMv, 1);
    Serial.print(F(",Code:"));   Serial.print(code);
    Serial.print(F(",MaxErr_mV:")); Serial.println(maxErrMv, 1);
  }
}
