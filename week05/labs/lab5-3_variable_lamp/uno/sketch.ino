/*
 * 실습 5-3  ADC → PWM 가변램프  —  Arduino UNO R3
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 5V / GND, Wiper → A0
 *        D9 → 330Ω → LED(+) → LED(-) → GND
 *        D2 ↔ 푸시버튼 ↔ GND   (선형 ↔ 감마 모드 전환)
 *
 * 흐름 : ADC 0~1023 → (선형 map 또는 감마 보정) → PWM 0~255 → Duty → LED 밝기
 *   - 선형 : PWM = map(ADC, 0, 1023, 0, 255)
 *   - 감마 : PWM = 255 × x^2.2  (x = ADC / 1023) — 사람 눈의 비선형 감각 보정
 *   - DEAD_ZONE : 0 근처 잡음 때문에 LED 가 미세하게 깜빡이는 것 방지
 *
 * 주기 : 20 ms(50 Hz) 마다 샘플링·출력,  200 ms 마다 시리얼 출력
 */

const int POT_PIN = A0;
const int LED_PIN = 9;                // PWM (Timer1)
const int BTN_PIN = 2;

const int ADC_MAX = 1023;
const int DEAD_ZONE = 8;              // 이 값 이하는 꺼짐
const float GAMMA = 2.2;

bool gammaMode = false;
int adc = 0, pwm = 0;
unsigned long lastSampleMs = 0, lastPrintMs = 0;

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

int adcToPwm(int a) {
  if (a <= DEAD_ZONE) return 0;                         // 잡음으로 인한 깜빡임 방지
  if (!gammaMode) return map(a, 0, ADC_MAX, 0, 255);    // 선형
  float x = a / (float)ADC_MAX;
  return (int)(pow(x, GAMMA) * 255.0 + 0.5);            // 감마 보정
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.println(F("# 실습 5-3 가변램프 (UNO) - 버튼: 선형/감마 전환"));
}

void loop() {
  if (buttonPressed()) {
    gammaMode = !gammaMode;
    Serial.print(F("# 모드 = ")); Serial.println(gammaMode ? F("GAMMA") : F("LINEAR"));
  }

  if (millis() - lastSampleMs >= 20) {                  // 50 Hz 샘플링
    lastSampleMs = millis();
    adc = analogRead(POT_PIN);
    pwm = adcToPwm(adc);
    analogWrite(LED_PIN, pwm);
  }

  if (millis() - lastPrintMs >= 200) {
    lastPrintMs = millis();
    float v = adc * 5.0 / 1024.0;
    float duty = pwm * 100.0 / 255.0;
    Serial.print(gammaMode ? F("GAMMA ") : F("LINEAR "));
    Serial.print(F("ADC=")); Serial.print(adc);
    Serial.print(F(" V=")); Serial.print(v, 2);
    Serial.print(F(" PWM=")); Serial.print(pwm);
    Serial.print(F(" Duty=")); Serial.print(duty, 1); Serial.println(F("%"));
  }
}
