/*
 * 터널 램프 T3 — 논블로킹 멜로디 재생/정지 테스트
 * 버튼(UNO D2 / ESP32 GPIO13)을 누를 때마다 멜로디 시작 ↔ 정지
 * 멜로디가 나오는 동안에도 Z1 LED 가 0.25초 간격으로 깜빡여야 한다
 *   → delay() 없이 millis() 로 음을 바꾸기 때문에 다른 일을 동시에 할 수 있다는 증거
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

const int MELODY[] = {262, 262, 392, 392, 440, 440, 392, 0,      // 반짝반짝 작은 별
                      349, 349, 330, 330, 294, 294, 262, 0};
const int MELODY_LEN = sizeof(MELODY) / sizeof(MELODY[0]);
const unsigned long BEAT_MS = 300;
bool melodyOn = false, ledOn = false;
int noteIdx = 0;
unsigned long noteStartMs = 0, lastBlinkMs = 0;

bool btnStable = HIGH, btnLastRead = HIGH;
unsigned long btnChangedMs = 0;
bool buttonPressed() {
  bool r = digitalRead(BTN_MUTE);
  if (r != btnLastRead) { btnLastRead = r; btnChangedMs = millis(); }
  if (millis() - btnChangedMs > 30 && r != btnStable) { btnStable = r; return btnStable == LOW; }
  return false;
}

void playNote() {
  int f = MELODY[noteIdx];
  if (f > 0) tone(BUZZER_PIN, f, BEAT_MS * 9 / 10);
  else noTone(BUZZER_PIN);
  Serial.print(F("음 ")); Serial.print(noteIdx); Serial.print(F(": ")); Serial.print(f); Serial.println(F(" Hz"));
}
void melodyStart() { melodyOn = true; noteIdx = 0; noteStartMs = millis(); playNote(); }
void melodyStop()  { melodyOn = false; noTone(BUZZER_PIN); }
void melodyUpdate(unsigned long now) {
  if (!melodyOn || now - noteStartMs < BEAT_MS) return;
  noteStartMs = now;
  noteIdx = (noteIdx + 1) % MELODY_LEN;
  playNote();
}

void setup() {
  Serial.begin(115200);
  pinMode(ZONE_PINS[0], OUTPUT);
  pinMode(BTN_MUTE, INPUT_PULLUP);
  Serial.println(F("# T3 멜로디 테스트: 버튼으로 시작/정지"));
}

void loop() {
  unsigned long now = millis();
  if (buttonPressed()) {
    if (melodyOn) { melodyStop(); Serial.println(F("# 멜로디 정지")); }
    else          { Serial.println(F("# 멜로디 시작")); melodyStart(); }
  }
  melodyUpdate(now);
  if (now - lastBlinkMs >= 250) { lastBlinkMs = now; ledOn = !ledOn; digitalWrite(ZONE_PINS[0], ledOn); }
}
