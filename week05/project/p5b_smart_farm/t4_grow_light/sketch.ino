/*
 * 스마트팜 T4 — 생장 LED(빨강+파랑) 일출·일몰 PWM 테스트
 * 하루를 40초로 줄여서: 0~5초 일출(서서히 밝아짐) → 5~20초 낮 → 20~25초 일몰 → 25~40초 밤
 */
#if defined(ESP32)
const int POT_PIN = 34, DHT_PIN = 4;
const int GROW_R = 25, GROW_B = 26, PUMP_LED = 16, FAN_LED = 17, ALARM_LED = 18;
const int BUZZER_PIN = 23;
const int BTN_PINS[4] = {13, 14, 32, 33};        // MODE, PUMP, FAN, MUTE
const int ADC_MAX = 4095;
#else
const int POT_PIN = A0, DHT_PIN = 4;
const int GROW_R = 9, GROW_B = 10, PUMP_LED = 7, FAN_LED = 8, ALARM_LED = 13;
const int BUZZER_PIN = 11;
const int BTN_PINS[4] = {2, 3, 5, 6};
const int ADC_MAX = 1023;
#endif

const unsigned long DAY_MS = 40000;
const unsigned long SUNRISE_END = 5000, SUNSET_START = 20000, SUNSET_END = 25000;
unsigned long lastPrintMs = 0;

int growLevel(unsigned long now) {
  unsigned long p = now % DAY_MS;
  if (p < SUNRISE_END)  return map(p, 0, SUNRISE_END, 0, 255);
  if (p < SUNSET_START) return 255;
  if (p < SUNSET_END)   return map(p, SUNSET_START, SUNSET_END, 255, 0);
  return 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(GROW_R, OUTPUT); pinMode(GROW_B, OUTPUT);
  Serial.println(F("# T4 생장 LED 테스트 (하루 = 40초)"));
}

void loop() {
  unsigned long now = millis();
  int pwm = growLevel(now);
  analogWrite(GROW_R, pwm);
  analogWrite(GROW_B, pwm);
  if (now - lastPrintMs >= 1000) {
    lastPrintMs = now;
    Serial.print(F("시각=")); Serial.print((now % DAY_MS) / 1000); Serial.print(F("s PWM="));
    Serial.print(pwm); Serial.println(pwm == 0 ? F(" (밤)") : (pwm == 255 ? F(" (낮)") : F(" (일출/일몰)")));
  }
}
