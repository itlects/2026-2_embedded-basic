/*
 * 실습 5-3  ADC → PWM 가변램프  —  ESP32 DevKitC V4
 * ------------------------------------------------------------------
 * 회로 : 가변저항 양 끝 3.3V / GND, Wiper → GPIO34   (5V 연결 금지)
 *        GPIO25 → 330Ω → LED(+) → LED(-) → GND
 *        GPIO13 ↔ 푸시버튼 ↔ GND   (선형 ↔ 감마 모드 전환)
 *
 * 흐름 : ADC 0~4095 → (선형 map 또는 감마 보정) → PWM 0~255 → LED 밝기
 *        analogWrite() 는 LEDC 를 사용 → Arduino-ESP32 코어 3.x 권장
 */

const int POT_PIN = 34;
const int LED_PIN = 25;
const int BTN_PIN = 13;

const int ADC_MAX = 4095;
const int DEAD_ZONE = 32;             // ESP32 는 0 근처 잡음이 커서 UNO 보다 크게
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
  if (a <= DEAD_ZONE) return 0;
  if (!gammaMode) return map(a, 0, ADC_MAX, 0, 255);
  float x = a / (float)ADC_MAX;
  return (int)(pow(x, GAMMA) * 255.0 + 0.5);
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  Serial.println("# 실습 5-3 가변램프 (ESP32) - 버튼: 선형/감마 전환");
}

void loop() {
  if (buttonPressed()) {
    gammaMode = !gammaMode;
    Serial.printf("# 모드 = %s\n", gammaMode ? "GAMMA" : "LINEAR");
  }

  if (millis() - lastSampleMs >= 20) {
    lastSampleMs = millis();
    adc = analogRead(POT_PIN);
    pwm = adcToPwm(adc);
    analogWrite(LED_PIN, pwm);
  }

  if (millis() - lastPrintMs >= 200) {
    lastPrintMs = millis();
    float v = analogReadMilliVolts(POT_PIN) / 1000.0;
    Serial.printf("%s ADC=%d V=%.2f PWM=%d Duty=%.1f%%\n",
                  gammaMode ? "GAMMA" : "LINEAR", adc, v, pwm, pwm * 100.0 / 255.0);
  }
}
