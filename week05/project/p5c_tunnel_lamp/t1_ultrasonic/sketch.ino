/*
 * 터널 램프 T1 — 초음파 센서(HC-SR04) 거리 측정 테스트
 * 원리: TRIG 에 10 us 펄스 → 초음파 발사 → 돌아온 시간(ECHO HIGH 길이, us) 측정
 *       거리(cm) = 시간(us) × 0.0343 ÷ 2   (음속 343 m/s, 왕복이므로 ÷2)
 * 측정 주기 60 ms : 앞선 초음파의 메아리가 사라질 시간 → 이것도 "샘플링 주기"이다.
 * Wokwi: 센서를 클릭해 거리 슬라이더를 움직인다.
 */
#if defined(ESP32)
const int TRIG_PIN = 5, ECHO_PIN = 19;
const int ZONE_PINS[3] = {25, 26, 27};
const int BUZZER_PIN = 23, POT_PIN = 34, BTN_MUTE = 13;
const int ADC_MAX = 4095;
#else
const int TRIG_PIN = 7, ECHO_PIN = 8;
const int ZONE_PINS[3] = {5, 6, 9};
const int BUZZER_PIN = 11, POT_PIN = A0, BTN_MUTE = 2;
const int ADC_MAX = 1023;
#endif

const unsigned long PING_MS = 60;
unsigned long lastPingMs = 0, lastPrintMs = 0;
long distCm = -1;

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);       // 10 us 트리거 펄스
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 12000UL);       // 최대 약 2 m 까지만 기다림
  if (us == 0) return -1;                                    // 반사 없음
  return (long)(us * 0.0343 / 2.0);                          // 음속 343 m/s, 왕복 ÷2
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  Serial.println(F("# T1 초음파 거리 측정 (60 ms 주기, 0.2초마다 출력)"));
}

void loop() {
  unsigned long now = millis();
  if (now - lastPingMs >= PING_MS) { lastPingMs = now; distCm = readDistanceCm(); }
  if (now - lastPrintMs >= 200) {
    lastPrintMs = now;
    if (distCm < 0) Serial.println(F("거리=--- (반사 없음 / 2 m 이상)"));
    else { Serial.print(F("거리=")); Serial.print(distCm); Serial.println(F(" cm")); }
  }
}
