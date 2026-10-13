/*
 * 터널 램프 T2 — 거리 → 구간(Z1~Z3) 양자화 + 구간 LED + 가변저항 밝기 테스트
 *   [입구] Z1: 70~100 cm | Z2: 40~70 cm | Z3: 10~40 cm [출구·센서]
 * 사람이 있는 구간의 LED(좌·우 2개)만 켜고, 밝기는 가변저항(ADC → PWM)으로 조절한다.
 * 이 단계에서는 "앞 구간 미리 켜기"와 멜로디 없이 구간 판정만 확인한다.
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

const int TUNNEL_CM = 100, EXIT_CM = 10;
const unsigned long PING_MS = 60;
unsigned long lastPingMs = 0, lastPrintMs = 0;
long distCm = -1;
int zone = -1, brightness = 255;

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);       // 10 us 트리거 펄스
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 12000UL);       // 최대 약 2 m 까지만 기다림
  if (us == 0) return -1;                                    // 반사 없음
  return (long)(us * 0.0343 / 2.0);                          // 음속 343 m/s, 왕복 ÷2
}

int zoneOf(long d) {
  if (d > 70) return 0;
  if (d > 40) return 1;
  return 2;
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  for (int z = 0; z < 3; z++) pinMode(ZONE_PINS[z], OUTPUT);
  Serial.println(F("# T2 구간 LED 테스트"));
}

void loop() {
  unsigned long now = millis();
  if (now - lastPingMs >= PING_MS) {
    lastPingMs = now;
    distCm = readDistanceCm();
    bool inTunnel = (distCm > EXIT_CM && distCm <= TUNNEL_CM);
    zone = inTunnel ? zoneOf(distCm) : -1;
    brightness = map(analogRead(POT_PIN), 0, ADC_MAX, 20, 255);
    for (int z = 0; z < 3; z++) analogWrite(ZONE_PINS[z], (z == zone) ? brightness : 0);
  }
  if (now - lastPrintMs >= 300) {
    lastPrintMs = now;
    Serial.print(F("거리=")); Serial.print(distCm); Serial.print(F("cm 구간="));
    if (zone < 0) Serial.print(F("터널 밖")); else { Serial.print('Z'); Serial.print(zone + 1); }
    Serial.print(F(" 밝기PWM=")); Serial.println(brightness);
  }
}
