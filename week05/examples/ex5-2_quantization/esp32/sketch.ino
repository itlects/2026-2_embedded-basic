/*
 * 실습 5-2  전압 환산 & 양자화 실험  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 5-1 과 동일 (가변저항 3.3V / GND, Wiper → GPIO34, 버튼 GPIO13 ↔ GND)
 *
 * 동작 : 12-bit 로 읽은 값을 오른쪽 시프트해서 8 / 6 / 4 / 2-bit ADC 를 흉내 낸다.
 *          V      : 12-bit 단순 환산 전압
 *          Vq     : b-bit 코드로 복원한 전압 (계단)
 *          Err_mV : 양자화 오차 V - Vq
 *          Vcal   : analogReadMilliVolts() — 공장 보정값을 적용한 전압
 *        V 와 Vcal 의 차이 = "해상도가 높아도 정확하지는 않다"(비선형 오차)
 *
 * 참고 : ESP32 ADC 의 실제 유효 범위는 약 0.15 ~ 3.1 V 이며 양 끝에서 포화된다.
 */

const int POT_PIN = 34;
const int BTN_PIN = 13;
const float VREF = 3.3;
const int ADC_BITS = 12;

const int BITS[] = {12, 8, 6, 4, 2};
const int BITS_COUNT = sizeof(BITS) / sizeof(BITS[0]);
int bitsIdx = 3;                                        // 4-bit 에서 시작

unsigned long lastSampleMs = 0;
float maxErrMv = 0;

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
  Serial.printf("# %d-bit : 단계 수 %ld, 1 LSB = %.2f mV, 최대 오차 ±%.2f mV\n",
                b, 1L << b, lsbMv, lsbMv / 2);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  Serial.println("# 실습 5-2 양자화 (ESP32) - 버튼으로 비트 수 변경");
  printInfo();
}

void loop() {
  if (buttonPressed()) {
    bitsIdx = (bitsIdx + 1) % BITS_COUNT;
    maxErrMv = 0;
    printInfo();
  }

  if (millis() - lastSampleMs >= 50) {
    lastSampleMs = millis();
    int raw  = analogRead(POT_PIN);                     // 0 ~ 4095
    float v  = (raw + 0.5) * VREF / 4096.0;
    int b    = BITS[bitsIdx];
    int code = raw >> (ADC_BITS - b);
    float lsb = VREF / (1L << b);
    float vq = (code + 0.5) * lsb;
    float errMv = (v - vq) * 1000.0;
    if (fabs(errMv) > maxErrMv) maxErrMv = fabs(errMv);
    float vcal = analogReadMilliVolts(POT_PIN) / 1000.0; // 보정된 전압

    Serial.printf("V:%.3f,Vq:%.3f,Vcal:%.3f,Err_mV:%.1f,Code:%d,MaxErr_mV:%.1f\n",
                  v, vq, vcal, errMv, code, maxErrMv);
  }
}
