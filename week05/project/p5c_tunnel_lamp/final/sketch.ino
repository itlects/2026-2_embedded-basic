/*
 * 터널 멜로디 거리 램프 워크북 — 완성 코드 (Arduino UNO / ESP32 공용)
 * ------------------------------------------------------------------
 * 터널 모형(길이 100 cm) 양쪽 벽에 LED 를 3구간(입구·중간·출구)으로 배치하고,
 * 출구 쪽 벽에 초음파 센서를 터널 안쪽을 향하게 설치한다 → 측정 거리 = 사람의 위치.
 *
 *   [입구]  Z1(70~100cm)  |  Z2(40~70cm)  |  Z3(10~40cm)  [출구·센서]
 *
 * 동작
 *   - 사람이 터널에 들어오면 멜로디 시작, 사람이 있는 구간 + 진행 방향 앞 구간 LED ON
 *   - 지나온 구간 LED 는 OFF
 *   - 반대편 끝으로 빠져나가면(통과) 멜로디 정지, LED 모두 OFF
 *   - 중간에 되돌아가 사라지면(1.5초간 감지 없음) 대기 상태로 복귀
 * 가변저항 = LED 밝기(ADC → PWM), 버튼 = 멜로디 음소거, I2C LCD = 거리·구간·상태 표시
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if defined(ESP32)
const int TRIG_PIN = 5, ECHO_PIN = 19;           // ECHO 는 5V → 분압 필요 (워크북 참고)
const int ZONE_PINS[3] = {25, 26, 27};           // Z1 입구, Z2 중간, Z3 출구 (각 핀에 좌·우 LED 2개)
const int BUZZER_PIN = 23, POT_PIN = 34, BTN_MUTE = 13;
const int ADC_MAX = 4095;
#else
const int TRIG_PIN = 7, ECHO_PIN = 8;
const int ZONE_PINS[3] = {5, 6, 9};              // PWM 핀 (D3·D11 은 tone() 때문에 PWM 불가)
const int BUZZER_PIN = 11, POT_PIN = A0, BTN_MUTE = 2;
const int ADC_MAX = 1023;
#endif

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- 터널 모형 ----------
const int TUNNEL_CM = 100;                       // 센서 ~ 입구 거리
const int EXIT_CM = 10;                          // 이보다 가까우면 출구 통과
const unsigned long PING_MS = 60;                // 초음파 측정 주기 (60 ms 이상 권장)
const unsigned long LOST_MS = 1500;              // 이 시간 동안 감지 없으면 사라진 것으로 판단
const unsigned long CLEAR_MS = 1000;             // 통과 후 터널이 비었다고 판단하는 시간

enum State { ST_IDLE, ST_IN, ST_PASSED };
const char* STATE_NAME[] = {"IDLE", "IN", "PASSED"};
State state = ST_IDLE;
int dir = +1;                                    // +1: 입구→출구(센서 쪽), -1: 출구→입구
int zone = -1;                                   // 0=Z1, 1=Z2, 2=Z3
long distCm = -1;
int brightness = 255, passCount = 0;
bool muted = false;
unsigned long lastPingMs = 0, lastSeenMs = 0, clearStartMs = 0, lastLcdMs = 0, lastPrintMs = 0;

// ---------- 멜로디 (반짝반짝 작은 별, 논블로킹 반복) ----------
const int MELODY[] = {262, 262, 392, 392, 440, 440, 392, 0,
                      349, 349, 330, 330, 294, 294, 262, 0};
const int MELODY_LEN = sizeof(MELODY) / sizeof(MELODY[0]);
const unsigned long BEAT_MS = 300;
bool melodyOn = false;
int noteIdx = 0;
unsigned long noteStartMs = 0;

void playNote() {
  int f = MELODY[noteIdx];
  if (f > 0 && !muted) tone(BUZZER_PIN, f, BEAT_MS * 9 / 10);   // 90 % 만 울려 음 사이를 끊음
  else noTone(BUZZER_PIN);
}
void melodyStart() { melodyOn = true; noteIdx = 0; noteStartMs = millis(); playNote(); }
void melodyStop()  { melodyOn = false; noTone(BUZZER_PIN); }
void melodyUpdate(unsigned long now) {
  if (!melodyOn || now - noteStartMs < BEAT_MS) return;
  noteStartMs = now;
  noteIdx = (noteIdx + 1) % MELODY_LEN;
  playNote();
}

// ---------- 버튼 ----------
bool btnStable = HIGH, btnLastRead = HIGH;
unsigned long btnChangedMs = 0;
bool mutePressed() {
  bool r = digitalRead(BTN_MUTE);
  if (r != btnLastRead) { btnLastRead = r; btnChangedMs = millis(); }
  if (millis() - btnChangedMs > 30 && r != btnStable) { btnStable = r; return btnStable == LOW; }
  return false;
}

// ---------- 초음파 거리 ----------
long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);       // 10 us 트리거 펄스
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 12000UL);       // 최대 약 2 m 까지만 기다림
  if (us == 0) return -1;                                    // 반사 없음
  return (long)(us * 0.0343 / 2.0);                          // 음속 343 m/s, 왕복 ÷2
}

int zoneOf(long d) {                             // 거리 → 구간 (양자화)
  if (d > 70) return 0;                          // Z1 입구
  if (d > 40) return 1;                          // Z2 중간
  return 2;                                      // Z3 출구
}

// ---------- 상태 전환 ----------
void enterTunnel(long d, unsigned long now) {
  zone = zoneOf(d);
  dir = (zone == 2) ? -1 : +1;                   // 센서 쪽에서 들어오면 반대 방향
  state = ST_IN; lastSeenMs = now;
  melodyStart();
  Serial.print(F("# 진입 ")); Serial.print(d); Serial.print(F("cm Z")); Serial.print(zone + 1);
  Serial.println(dir > 0 ? F(" 방향: 입구→출구") : F(" 방향: 출구→입구"));
}

void passTunnel(unsigned long now) {
  state = ST_PASSED; clearStartMs = now; passCount++;
  melodyStop();
  Serial.print(F("# 통과 완료! 멜로디 정지 (누적 ")); Serial.print(passCount); Serial.println(F("명)"));
}

void lostTunnel() {
  state = ST_IDLE; zone = -1;
  melodyStop();
  Serial.println(F("# 감지 사라짐 → 대기 (되돌아감)"));
}

void updateState(unsigned long now) {
  bool inTunnel = (distCm > EXIT_CM && distCm <= TUNNEL_CM);
  switch (state) {
    case ST_IDLE:
      if (inTunnel) enterTunnel(distCm, now);
      break;
    case ST_IN:
      if (inTunnel) {
        int z = zoneOf(distCm);
        if (z != zone) { zone = z; Serial.print(F("# 구간 Z")); Serial.println(zone + 1); }
        lastSeenMs = now;
      } else if (dir > 0 && distCm > 0 && distCm <= EXIT_CM) {
        passTunnel(now);                                       // 출구(센서 앞)에 도착
      } else if (dir < 0 && distCm > TUNNEL_CM) {
        passTunnel(now);                                       // 입구 밖으로 나감
      } else if (now - lastSeenMs >= LOST_MS) {
        int exitZone = (dir > 0) ? 2 : 0;
        if (zone == exitZone) passTunnel(now); else lostTunnel();
      }
      break;
    case ST_PASSED:
      if (inTunnel) clearStartMs = now;                        // 아직 터널 안에 무언가 있음
      else if (now - clearStartMs >= CLEAR_MS) { state = ST_IDLE; zone = -1; Serial.println(F("# 대기")); }
      break;
  }
}

// ---------- 출력 ----------
void updateLeds() {
  for (int z = 0; z < 3; z++) {
    bool on = (state == ST_IN) && (z == zone || z == zone + dir);   // 현재 구간 + 앞 구간
    analogWrite(ZONE_PINS[z], on ? brightness : 0);
  }
}

void lcdLine(uint8_t row, const char* text) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", text);
  lcd.setCursor(0, row); lcd.print(buf);
}

void updateLcd() {
  char l1[17], l2[17], st[16];
  if (distCm < 0) snprintf(l1, sizeof(l1), "D: ---cm");
  else if (state == ST_IN) snprintf(l1, sizeof(l1), "D:%4ldcm  Z%d", distCm, zone + 1);
  else snprintf(l1, sizeof(l1), "D:%4ldcm", distCm);
  if (state == ST_IN)          snprintf(st, sizeof(st), "IN %s", dir > 0 ? ">> EXIT" : "<< ENTR");
  else if (state == ST_PASSED) snprintf(st, sizeof(st), "PASSED! #%d", passCount);
  else                         snprintf(st, sizeof(st), "IDLE    #%d", passCount);
  snprintf(l2, sizeof(l2), "%-15s%c", st, muted ? 'M' : ' ');
  lcdLine(0, l1); lcdLine(1, l2);
}

void setup() {
  Serial.begin(115200);
#if defined(ESP32)
  analogReadResolution(12);
#endif
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  for (int z = 0; z < 3; z++) pinMode(ZONE_PINS[z], OUTPUT);
  pinMode(BTN_MUTE, INPUT_PULLUP);
  lcd.init(); lcd.backlight();
  lcdLine(0, "Tunnel Lamp");
  lcdLine(1, "Week5 Workbook");
  Serial.println(F("# 터널 멜로디 거리 램프 완성 코드"));
  delay(1000);                                   // 시작 화면 (setup 에서만 사용)
}

void loop() {
  unsigned long now = millis();

  if (mutePressed()) {
    muted = !muted;
    if (muted) noTone(BUZZER_PIN);
    Serial.print(F("# 음소거 ")); Serial.println(muted ? F("ON") : F("OFF"));
  }

  if (now - lastPingMs >= PING_MS) {             // 60 ms 마다 거리 샘플링
    lastPingMs = now;
    distCm = readDistanceCm();
    brightness = map(analogRead(POT_PIN), 0, ADC_MAX, 20, 255);
    updateState(now);
  }

  melodyUpdate(now);
  updateLeds();

  if (now - lastLcdMs >= 300) { lastLcdMs = now; updateLcd(); }
  if (now - lastPrintMs >= 500) {
    lastPrintMs = now;
    Serial.print(F("d=")); Serial.print(distCm); Serial.print(F("cm 상태=")); Serial.print(STATE_NAME[state]);
    Serial.print(F(" 구간=")); if (zone < 0) Serial.print('-'); else { Serial.print('Z'); Serial.print(zone + 1); }
    Serial.print(F(" 밝기=")); Serial.println(brightness);
  }
}
